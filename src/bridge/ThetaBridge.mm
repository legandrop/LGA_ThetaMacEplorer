// ThetaBridge.mm
// Objective-C++ implementation of the Qt <-> ImageCaptureCore bridge.
// This is the ONLY file that imports Objective-C/ImageCaptureCore headers.
//
// KEY DESIGN NOTE — debounce pattern:
// ImageCaptureCore fires many delegate callbacks in rapid succession during
// camera init (didAddItems, cameraDeviceDidChangeCapability, didReceiveSessionOptions,
// deviceDidBecomeReadyWithCompleteContentCatalog, ...).  If we call enumerateFiles
// directly from each one we get a burst of fileListUpdated signals that causes the
// Qt grid to be rebuilt 5-6 times, the thumbnail timer to be restarted each time,
// and ultimately a PTP command-queue flood that hangs the camera.
//
// Solution: scheduleEnumeration() cancels any pending GCD block and posts a new one
// 300 ms into the future.  The burst collapses into a single enumeration.

#import "ThetaBridgePrivate.h"
#include "thetaexplorer/Logger.h"
#include <QMetaObject>
#include <QPixmap>
#include <QImage>
#include <CoreGraphics/CoreGraphics.h>
#include <vector>

// ============================================================
// ThetaBridgePrivate: holds all Objective-C objects
// ============================================================
class ThetaBridgePrivate {
public:
    ICDeviceBrowser*            browser    = nil;
    ThetaDeviceBrowserAdapter*  browserDel = nil;
    ThetaCameraDeviceAdapter*   cameraDel  = nil;
};

// ============================================================
// ThetaBridge (QObject) - method implementations
// ============================================================
ThetaBridge::ThetaBridge(QObject* parent)
    : QObject(parent)
    , d(new ThetaBridgePrivate)
{}

ThetaBridge::~ThetaBridge()
{
    stopBrowsing();
    delete d;
}

void ThetaBridge::startBrowsing()
{
    d->browserDel           = [[ThetaDeviceBrowserAdapter alloc] init];
    d->browserDel.qtBridge  = this;

    d->browser              = [[ICDeviceBrowser alloc] init];
    d->browser.delegate     = d->browserDel;
    // Must cast because browsedDeviceTypeMask is ICDeviceTypeMask (NS_ENUM NSUInteger)
    // and the OR of two NS_ENUMs produces plain NSUInteger
    d->browser.browsedDeviceTypeMask =
        (ICDeviceTypeMask)(ICDeviceTypeMaskCamera | ICDeviceLocationTypeMaskLocal);
    [d->browser start];
    LOGI("bridge") << "startBrowsing";
}

void ThetaBridge::stopBrowsing()
{
    if (d->browser) {
        [d->browser stop];
        d->browser.delegate = nil;
        d->browser = nil;
    }
}

void ThetaBridge::refreshCatalog()
{
    if (d->cameraDel && d->cameraDel.camera) {
        LOGI("bridge") << "refreshCatalog on active session";
        [d->cameraDel scheduleEnumeration];
        return;
    }

    LOGI("bridge") << "refreshCatalog without active session; restarting browser";
    stopBrowsing();
    startBrowsing();
}

void ThetaBridge::recoverDownloadSession()
{
    if (d->cameraDel && d->cameraDel.camera) {
        LOGW("bridge") << "recoverDownloadSession requested";
        [d->cameraDel recoverSessionForDownloads];
        return;
    }

    LOGW("bridge") << "recoverDownloadSession without active camera; restarting browser";
    stopBrowsing();
    startBrowsing();
}

void ThetaBridge::requestThumbnail(const CameraFileInfo& file)
{
    if (!d->cameraDel || !d->cameraDel.camera) return;
    ICCameraFile* icFile = (__bridge ICCameraFile*)file.nativeFileHandle;
    if (icFile) [icFile requestThumbnail];
}

void ThetaBridge::requestDownloadFile(const CameraFileInfo& file,
                                      const QString& destinationPath)
{
    if (!d->cameraDel || !d->cameraDel.camera) {
        LOGW("bridge") << "requestDownloadFile: no camera";
        return;
    }

    ICCameraFile* icFile = (__bridge ICCameraFile*)file.nativeFileHandle;
    if (!icFile) {
        LOGW("bridge") << "requestDownloadFile: invalid file handle for" << file.name;
        return;
    }

    ICCameraDevice* cam = d->cameraDel.camera;
    NSURL* destURL = [NSURL fileURLWithPath:destinationPath.toNSString()];
    NSDictionary* options = @{
        ICDownloadsDirectoryURL:         destURL,
        ICOverwrite:                     @YES,
        ICDeleteAfterSuccessfulDownload: @NO
    };
    [cam requestDownloadFile:icFile
                     options:options
            downloadDelegate:d->cameraDel
         didDownloadSelector:@selector(didDownloadFile:error:options:contextInfo:)
                 contextInfo:NULL];
    LOGD("bridge") << "Queued download:"
                   << file.name
                   << "destDir:" << destinationPath
                   << "sizeBytes:" << file.sizeBytes
                   << "devicePath:" << file.devicePath;
}

void ThetaBridge::requestDownloadFiles(const QList<CameraFileInfo>& files,
                                        const QString& destinationPath)
{
    for (const CameraFileInfo& fi : files) {
        requestDownloadFile(fi, destinationPath);
    }
}

void ThetaBridge::requestDeleteFiles(const QList<CameraFileInfo>& files)
{
    if (!d->cameraDel || !d->cameraDel.camera) return;
    NSMutableArray<ICCameraItem*>* icFiles = [NSMutableArray array];
    for (const CameraFileInfo& fi : files) {
        ICCameraFile* icFile = (__bridge ICCameraFile*)fi.nativeFileHandle;
        if (icFile) [icFiles addObject:icFile];
    }
    if (icFiles.count > 0) {
        [d->cameraDel.camera requestDeleteFiles:icFiles];
        LOGD("bridge") << "requestDeleteFiles:" << (int)icFiles.count;
    }
}

// ============================================================
// Utility: CGImageRef → QPixmap
// ============================================================
static QPixmap cgImageToQPixmap(CGImageRef cgImg)
{
    if (!cgImg) return QPixmap();
    size_t w = CGImageGetWidth(cgImg);
    size_t h = CGImageGetHeight(cgImg);
    if (w == 0 || h == 0) return QPixmap();

    CGColorSpaceRef cs  = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    size_t bpr          = w * 4;
    std::vector<uint8_t> buf(h * bpr, 0);
    CGContextRef ctx = CGBitmapContextCreate(
        buf.data(), w, h, 8, bpr, cs,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host
    );
    CGColorSpaceRelease(cs);
    if (!ctx) return QPixmap();

    CGContextDrawImage(ctx, CGRectMake(0, 0, (CGFloat)w, (CGFloat)h), cgImg);
    CGContextRelease(ctx);

    // kCGBitmapByteOrder32Host + kCGImageAlphaPremultipliedFirst on little-endian = BGRA
    QImage img(buf.data(), (int)w, (int)h, (int)bpr,
               QImage::Format_ARGB32_Premultiplied);
    return QPixmap::fromImage(img.copy());
}

// ============================================================
// ThetaDeviceBrowserAdapter
// ============================================================
@implementation ThetaDeviceBrowserAdapter

- (void)deviceBrowser:(ICDeviceBrowser*)browser
         didAddDevice:(ICDevice*)addedDevice
           moreComing:(BOOL)moreComing
{
    NSString* devName = addedDevice.name ?: @"(unnamed)";
    NSString* devType = NSStringFromClass([addedDevice class]);
    LOGD("bridge") << "Device found:"
             << QString::fromNSString(devName)
             << "class:" << QString::fromNSString(devType)
             << "moreComing:" << moreComing;

    if (![addedDevice isKindOfClass:[ICCameraDevice class]]) {
        LOGD("bridge") << "not an ICCameraDevice, skipping";
        return;
    }

    ICCameraDevice* cam = (ICCameraDevice*)addedDevice;

    // ---- Device name filter ----
    // Only connect to the Ricoh Theta, not SD cards or other cameras.
    // The Theta always has "THETA" or "RICOH" in its device name.
    NSString* upperName = devName.uppercaseString;
    BOOL isTheta = [upperName containsString:@"THETA"] ||
                   [upperName containsString:@"RICOH"];

    if (!isTheta) {
        LOGD("bridge") << "not a Ricoh Theta (name="
                       << QString::fromNSString(devName) << "), skipping";
        return;
    }

    LOGI("bridge") << "Ricoh Theta identified. Connecting...";

    ThetaCameraDeviceAdapter* camDel = [[ThetaCameraDeviceAdapter alloc] init];
    camDel.qtBridge = self.qtBridge;
    camDel.camera   = cam;
    self.qtBridge->d->cameraDel = camDel;

    cam.delegate = camDel;
    [cam requestOpenSession];

    QString name = QString::fromNSString(cam.name);
    QMetaObject::invokeMethod(self.qtBridge, "cameraConnected",
                              Qt::QueuedConnection,
                              Q_ARG(QString, name));
}

- (void)deviceBrowser:(ICDeviceBrowser*)browser
      didRemoveDevice:(ICDevice*)device
            moreGoing:(BOOL)moreGoing
{
    LOGI("bridge") << "Camera removed";
    QMetaObject::invokeMethod(self.qtBridge, "cameraDisconnected",
                              Qt::QueuedConnection);
}

- (void)deviceBrowserDidEnumerateLocalDevices:(ICDeviceBrowser*)browser
{
    LOGD("bridge") << "Local device enumeration complete";
}

@end

// ============================================================
// ThetaCameraDeviceAdapter
// ============================================================
@implementation ThetaCameraDeviceAdapter

// ---- Debounce helper ----
// Cancels any pending enumeration and schedules a new one 300ms from now.
// All callbacks that used to call [self enumerateFiles] now call this instead.
- (void)scheduleEnumeration
{
    if (self.pendingEnumeration) {
        dispatch_block_cancel(self.pendingEnumeration);
    }
    __weak typeof(self) weakSelf = self;
    dispatch_block_t block = dispatch_block_create(DISPATCH_BLOCK_INHERIT_QOS_CLASS, ^{
        [weakSelf enumerateFiles];
    });
    self.pendingEnumeration = block;
    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, (int64_t)(300 * NSEC_PER_MSEC)),
        dispatch_get_main_queue(),
        block
    );
}

- (void)recoverSessionForDownloads
{
    if (!self.camera) {
        LOGW("bridge") << "recoverSessionForDownloads: no camera";
        return;
    }

    if (self.recoveringDownloadSession || self.reopenSessionAfterClose) {
        LOGD("bridge") << "recoverSessionForDownloads already in progress";
        return;
    }

    LOGW("bridge") << "Closing session to recover download pipeline";
    self.recoveringDownloadSession = YES;
    self.reopenSessionAfterClose = YES;
    [self.camera requestCloseSession];
}

// ---- ICDeviceDelegate REQUIRED ----

- (void)device:(ICDevice*)device didOpenSessionWithError:(NSError* _Nullable)error
{
    if (error) {
        LOGW("bridge") << "Session open error:"
                   << QString::fromNSString(error.localizedDescription);
        self.recoveringDownloadSession = NO;
        self.reopenSessionAfterClose = NO;
        QMetaObject::invokeMethod(self.qtBridge, "cameraError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, QString::fromNSString(error.localizedDescription)));
    } else {
        LOGI("bridge") << "Session opened OK; scheduling enumeration";
        if (self.recoveringDownloadSession) {
            LOGI("bridge") << "Download session recovery completed";
            self.recoveringDownloadSession = NO;
        }
        // NOTE: do NOT call requestEnableTethering here.
        // It triggers cameraDeviceDidChangeCapability: callbacks which
        // were causing the burst-enumeration hang.
        [self scheduleEnumeration];
    }
}

- (void)device:(ICDevice*)device didCloseSessionWithError:(NSError* _Nullable)error
{
    LOGI("bridge") << "Session closed";
    if (self.pendingEnumeration) {
        dispatch_block_cancel(self.pendingEnumeration);
        self.pendingEnumeration = nil;
    }
    if (self.reopenSessionAfterClose && self.camera) {
        self.reopenSessionAfterClose = NO;
        LOGI("bridge") << "Reopening session after recovery close";
        dispatch_after(
            dispatch_time(DISPATCH_TIME_NOW, (int64_t)(350 * NSEC_PER_MSEC)),
            dispatch_get_main_queue(),
            ^{
                [self.camera requestOpenSession];
            }
        );
    }
}

- (void)didRemoveDevice:(ICDevice*)device
{
    LOGI("bridge") << "Device removed";
    if (self.pendingEnumeration) {
        dispatch_block_cancel(self.pendingEnumeration);
        self.pendingEnumeration = nil;
    }
    self.reopenSessionAfterClose = NO;
    self.recoveringDownloadSession = NO;
    QMetaObject::invokeMethod(self.qtBridge, "cameraDisconnected",
                              Qt::QueuedConnection);
}

// ---- ICDeviceDelegate optional ----

- (void)device:(ICDevice*)device didEncounterError:(NSError*)error
{
    QString msg = QString::fromNSString(error.localizedDescription);
    LOGW("bridge") << "Device error:" << msg;
    QMetaObject::invokeMethod(self.qtBridge, "cameraError",
                              Qt::QueuedConnection,
                              Q_ARG(QString, msg));
}

// ---- ICCameraDeviceDelegate REQUIRED ----

- (void)cameraDevice:(ICCameraDevice*)camera
    didReceiveThumbnail:(CGImageRef _Nullable)thumbnail
               forItem:(ICCameraItem*)item
                 error:(NSError* _Nullable)error
{
    if (!thumbnail || error) {
        if (error) {
            LOGW("bridge") << "Thumbnail error for" << QString::fromNSString(item.name)
                       << ":" << QString::fromNSString(error.localizedDescription);
        }
        return;
    }
    QPixmap pixmap = cgImageToQPixmap(thumbnail);
    if (pixmap.isNull()) return;

    NSString* pathNS = item.fileSystemPath;
    QString path = pathNS ? QString::fromNSString(pathNS)
                           : QString::fromNSString(item.name);

    QMetaObject::invokeMethod(self.qtBridge, "thumbnailReady",
                              Qt::QueuedConnection,
                              Q_ARG(QString, path),
                              Q_ARG(QPixmap, pixmap));
}

- (void)cameraDevice:(ICCameraDevice*)camera didRenameItems:(NSArray<ICCameraItem*>*)items
{
    LOGD("bridge") << "Items renamed";
    [self scheduleEnumeration];
}

- (void)cameraDevice:(ICCameraDevice*)camera didReceivePTPEvent:(NSData*)eventData
{
    // PTP pass-through events not needed for file browsing
}

// ---- ICCameraDeviceDelegate optional ----

- (void)cameraDeviceDidChangeCapability:(ICCameraDevice*)camera
{
    LOGD("bridge") << "Capability changed; scheduling enumeration";
    [self scheduleEnumeration];
}

- (void)cameraDevice:(ICCameraDevice*)camera didAddItems:(NSArray<ICCameraItem*>*)items
{
    LOGD("bridge") << "Items added:" << (int)items.count << "; scheduling enumeration";
    [self scheduleEnumeration];
}

- (void)cameraDevice:(ICCameraDevice*)camera didRemoveItems:(NSArray<ICCameraItem*>*)items
{
    LOGD("bridge") << "Items removed:" << (int)items.count;
    [self scheduleEnumeration];
}

- (void)deviceDidBecomeReadyWithCompleteContentCatalog:(ICCameraDevice*)device
{
    LOGD("bridge") << "Catalog complete; scheduling enumeration";
    [self scheduleEnumeration];
}

- (void)cameraDevice:(ICCameraDevice*)camera
    didReceiveSessionOptions:(NSDictionary<NSString*,id>*)options
{
    LOGD("bridge") << "Session options received; scheduling enumeration";
    [self scheduleEnumeration];
}

// Stub to suppress compiler warnings — we don't need metadata for file listing
- (void)cameraDevice:(ICCameraDevice*)camera
    didReceiveMetadata:(NSDictionary* _Nullable)metadata
               forItem:(ICCameraItem*)item
                 error:(NSError* _Nullable)error
{
    // Metadata arrives asynchronously. We don't request it explicitly,
    // but the camera sends it anyway. Ignore.
}

- (void)cameraDeviceDidRemoveAccessRestriction:(ICDevice*)device { }
- (void)cameraDeviceDidEnableAccessRestriction:(ICDevice*)device { }

// ---- File enumeration (runs once, after debounce settles) ----

- (void)enumerateFiles
{
    self.pendingEnumeration = nil;
    if (!self.camera) return;

    NSArray<ICCameraItem*>* allItems = self.camera.mediaFiles;
    if (!allItems) allItems = @[];

    QList<CameraFileInfo> fileList;
    fileList.reserve((int)allItems.count);

    for (ICCameraItem* item in allItems) {
        if (![item isKindOfClass:[ICCameraFile class]]) continue;
        ICCameraFile* f = (ICCameraFile*)item;

        CameraFileInfo info;
        info.name       = QString::fromNSString(f.name);
        info.uti        = f.UTI ? QString::fromNSString(f.UTI) : QString();

        // fileSystemPath is the macOS-side unique identifier for the file on the device
        NSString* syspath   = f.fileSystemPath;
        info.devicePath     = syspath ? QString::fromNSString(syspath)
                                      : QString::fromNSString(f.name);

        info.sizeBytes      = (qint64)f.fileSize;
        // width/height: don't access metadataIfAvailable here — it triggers extra
        // callbacks that can restart the enumeration cascade.
        // We'll get dimensions from EXIF after thumbnail arrives.
        info.width  = 0;
        info.height = 0;

        NSString* uti = f.UTI ?: @"";
        NSString* ext = f.name.pathExtension.lowercaseString ?: @"";

        info.isVideo = [uti hasPrefix:@"public.movie"]        ||
                       [uti isEqualToString:@"public.mpeg-4"] ||
                       [uti hasPrefix:@"public.video"]         ||
                       [ext isEqualToString:@"mp4"]            ||
                       [ext isEqualToString:@"mov"];

        info.isRaw   = [uti isEqualToString:@"com.adobe.raw-image"] ||
                       [uti hasSuffix:@"raw-image"]                   ||
                       [ext isEqualToString:@"dng"]                   ||
                       [ext isEqualToString:@"arw"]                   ||
                       [ext isEqualToString:@"cr2"]                   ||
                       [ext isEqualToString:@"nef"];

        if (f.creationDate) {
            NSTimeInterval ti = [f.creationDate timeIntervalSince1970];
            info.creationDate = QDateTime::fromSecsSinceEpoch((qint64)ti);
        }

        // Store raw Obj-C pointer as void*.
        // ARC keeps ICCameraFile alive through camera.mediaFiles (NSArray retains it).
        info.nativeFileHandle = (__bridge void*)f;
        fileList.append(info);
    }

    LOGI("bridge") << "enumerateFiles emitting" << fileList.size() << "files";
    QMetaObject::invokeMethod(self.qtBridge, "fileListUpdated",
                              Qt::QueuedConnection,
                              Q_ARG(QList<CameraFileInfo>, fileList));
}

// ---- Download progress ----

- (void)cameraDevice:(ICCameraDevice*)camera
    didReceiveDownloadProgressForFile:(ICCameraFile*)file
                      downloadedBytes:(off_t)downloadedBytes
                             maxBytes:(off_t)maxBytes
{
    int pct = (maxBytes > 0) ? (int)(downloadedBytes * 100 / maxBytes) : 0;
    QString name = QString::fromNSString(file.name);
    QMetaObject::invokeMethod(self.qtBridge, "downloadProgress",
                              Qt::QueuedConnection,
                              Q_ARG(QString, name),
                              Q_ARG(int, pct));
}

// ---- Download completion selector ----

- (void)didDownloadFile:(ICCameraFile*)file
                  error:(NSError*)error
                options:(NSDictionary*)options
            contextInfo:(void*)contextInfo
{
    QString name = QString::fromNSString(file.name);
    NSString* savedName = options ? options[ICSavedFilename] : nil;
    NSString* savedDir = nil;
    id dirUrl = options ? options[ICDownloadsDirectoryURL] : nil;
    if ([dirUrl isKindOfClass:[NSURL class]]) {
        savedDir = [(NSURL*)dirUrl path];
    }
    if (error) {
        QString msg = QString::fromNSString(error.localizedDescription);
        LOGW("bridge") << "Download error"
                       << name
                       << "savedName:" << (savedName ? QString::fromNSString(savedName) : QString())
                       << "destDir:" << (savedDir ? QString::fromNSString(savedDir) : QString())
                       << "code:" << error.code
                       << "domain:" << QString::fromNSString(error.domain ?: @"")
                       << "message:" << msg;
        QMetaObject::invokeMethod(self.qtBridge, "downloadError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, name),
                                  Q_ARG(QString, msg));
    } else {
        QString savedPath   = savedName ? QString::fromNSString(savedName) : QString();
        LOGI("bridge") << "Downloaded:"
                       << name
                       << "->" << savedPath
                       << "destDir:" << (savedDir ? QString::fromNSString(savedDir) : QString());
        QMetaObject::invokeMethod(self.qtBridge, "downloadFileCompleted",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, name),
                                  Q_ARG(QString, savedPath));
    }
}

// ---- Delete completion ----

- (void)cameraDevice:(ICCameraDevice*)camera
      didDeleteFiles:(NSArray<ICCameraItem*>*)deletedFiles
               error:(NSError*)error
{
    if (error) {
        QString msg = QString::fromNSString(error.localizedDescription);
        LOGW("bridge") << "Delete error:" << msg;
        QMetaObject::invokeMethod(self.qtBridge, "deleteError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, msg));
    } else {
        QStringList paths;
        for (ICCameraItem* item in deletedFiles) {
            NSString* p = item.fileSystemPath;
            paths << (p ? QString::fromNSString(p) : QString::fromNSString(item.name));
        }
        LOGI("bridge") << "Deleted:" << paths;
        QMetaObject::invokeMethod(self.qtBridge, "deleteCompleted",
                                  Qt::QueuedConnection,
                                  Q_ARG(QStringList, paths));
    }
}

@end
