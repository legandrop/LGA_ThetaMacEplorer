#include "thetaexplorer/ThetaCameraService.h"
#include "thetaexplorer/ThetaBridge.h"
#include "thetaexplorer/Logger.h"
#include <QTimer>

ThetaCameraService::ThetaCameraService(QObject* parent)
    : QObject(parent)
{
    m_bridge = new ThetaBridge(this);

    connect(m_bridge, &ThetaBridge::cameraConnected,
            this, [this](const QString& name) {
                m_connected = true;
                emit cameraConnected(name);
            });

    connect(m_bridge, &ThetaBridge::cameraDisconnected,
            this, [this]() {
                m_connected = false;
                m_files.clear();
                if (m_thumbnailTimer) m_thumbnailTimer->stop();
                m_thumbnailQueue.clear();
                m_downloadQueue.clear();
                m_activeDownload = PendingDownload{};
                m_downloadActive = false;
                m_recoveringAfterDownloadError = false;
                emit batteryLevelChanged(-1, false);
                emit cameraDisconnected();
            });

    connect(m_bridge, &ThetaBridge::fileListUpdated,
            this, &ThetaCameraService::onFileListUpdated);

    connect(m_bridge, &ThetaBridge::thumbnailReady,
            this, &ThetaCameraService::thumbnailReady);

    connect(m_bridge, &ThetaBridge::batteryLevelChanged,
            this, &ThetaCameraService::batteryLevelChanged);

    connect(m_bridge, &ThetaBridge::downloadProgress,
            this, &ThetaCameraService::onBridgeDownloadProgress);

    connect(m_bridge, &ThetaBridge::downloadFileCompleted,
            this, &ThetaCameraService::onBridgeDownloadFileCompleted);

    connect(m_bridge, &ThetaBridge::downloadError,
            this, &ThetaCameraService::onBridgeDownloadError);

    connect(m_bridge, &ThetaBridge::deleteCompleted,
            this, &ThetaCameraService::deleteCompleted);

    connect(m_bridge, &ThetaBridge::deleteError,
            this, [this](const QString& msg) {
                emit errorOccurred("Delete error: " + msg);
            });

    connect(m_bridge, &ThetaBridge::cameraError,
            this, [this](const QString& msg) {
                emit errorOccurred("Camera error: " + msg);
            });

    // Thumbnail drain timer
    m_thumbnailTimer = new QTimer(this);
    m_thumbnailTimer->setInterval(60); // ms between batches
    connect(m_thumbnailTimer, &QTimer::timeout,
            this, &ThetaCameraService::drainThumbnailQueue);
}

ThetaCameraService::~ThetaCameraService()
{
    stop();
}

void ThetaCameraService::start()
{
    LOGI("service") << "Starting camera browsing";
    m_bridge->startBrowsing();
}

void ThetaCameraService::stop()
{
    if (m_thumbnailTimer) m_thumbnailTimer->stop();
    m_downloadQueue.clear();
    m_activeDownload = PendingDownload{};
    m_downloadActive = false;
    m_recoveringAfterDownloadError = false;
    m_bridge->stopBrowsing();
}

void ThetaCameraService::refresh()
{
    LOGI("service") << "Refreshing camera browsing";
    if (m_thumbnailTimer) m_thumbnailTimer->stop();
    m_thumbnailQueue.clear();
    m_bridge->refreshCatalog();
}

void ThetaCameraService::onFileListUpdated(const QList<CameraFileInfo>& files)
{
    // The bridge already debounces (300ms GCD) so this should fire only once
    // per connection burst. We always accept the new list.
    LOGI("service") << "fileListUpdated:" << files.size() << "files";

    m_files = files;
    if (m_recoveringAfterDownloadError) {
        LOGI("service") << "Catalog refreshed during download recovery";
        QTimer::singleShot(250, this, &ThetaCameraService::resumeAfterRecovery);
    }

    // Stop any in-progress thumbnail fetching — stale handles after re-enumeration
    if (m_thumbnailTimer) m_thumbnailTimer->stop();
    m_thumbnailQueue.clear();

    emit fileListReady(m_files);

    // Queue all files for thumbnail fetching (throttled by timer)
    m_thumbnailQueue = m_files;
    if (!m_thumbnailQueue.isEmpty()) {
        m_thumbnailTimer->start();
    }
}

void ThetaCameraService::drainThumbnailQueue()
{
    // Request 3 thumbnails per tick to avoid overwhelming the camera
    for (int i = 0; i < 3 && !m_thumbnailQueue.isEmpty(); ++i) {
        CameraFileInfo fi = m_thumbnailQueue.takeFirst();
        m_bridge->requestThumbnail(fi);
    }
    if (m_thumbnailQueue.isEmpty()) {
        m_thumbnailTimer->stop();
        LOGD("service") << "All thumbnails requested";
    }
}

void ThetaCameraService::requestThumbnail(const CameraFileInfo& file)
{
    m_bridge->requestThumbnail(file);
}

void ThetaCameraService::downloadFiles(const QList<CameraFileInfo>& files,
                                        const QString& destinationPath)
{
    for (const CameraFileInfo& file : files) {
        m_downloadQueue.append(PendingDownload{file, destinationPath, 1});
        LOGD("service") << "Enqueued download candidate:"
                        << file.name
                        << "destDir:" << destinationPath
                        << "sizeBytes:" << file.sizeBytes
                        << "created:" << file.creationDate.toString(Qt::ISODate);
    }

    LOGI("service") << "Queued" << files.size() << "downloads for"
                    << destinationPath << "pending total:" << m_downloadQueue.size();
    if (!m_downloadActive) {
        kickNextDownload();
    }
}

void ThetaCameraService::deleteFiles(const QList<CameraFileInfo>& files)
{
    m_bridge->requestDeleteFiles(files);
}

void ThetaCameraService::onBridgeDownloadProgress(const QString& fileName, int percent)
{
    emit downloadProgress(fileName, percent);
}

void ThetaCameraService::onBridgeDownloadFileCompleted(const QString& fileName,
                                                       const QString& savedPath)
{
    if (!m_activeDownload.file.devicePath.isEmpty()) {
        m_failureCountByDevicePath.remove(m_activeDownload.file.devicePath);
    }
    emit downloadFileCompleted(fileName, savedPath);

    m_downloadActive = false;
    m_activeDownload = PendingDownload{};
    QTimer::singleShot(m_interDownloadDelayMs, this, &ThetaCameraService::kickNextDownload);
}

void ThetaCameraService::onBridgeDownloadError(const QString& fileName,
                                               const QString& errorMessage)
{
    const QString devicePath = m_activeDownload.file.devicePath;
    const int failureCount = devicePath.isEmpty()
        ? 0
        : (m_failureCountByDevicePath.value(devicePath, 0) + 1);
    if (!devicePath.isEmpty()) {
        m_failureCountByDevicePath.insert(devicePath, failureCount);
    }

    LOGW("service") << "Download failed:"
                    << fileName
                    << "destDir:" << m_activeDownload.destinationPath
                    << "attempt:" << m_activeDownload.attempt
                    << "failureCount:" << failureCount
                    << "sizeBytes:" << m_activeDownload.file.sizeBytes
                    << "devicePath:" << m_activeDownload.file.devicePath
                    << "message:" << errorMessage;

    const bool isPipelineError = errorMessage.contains("-9934");
    const bool canRecover = isPipelineError
        && m_activeDownload.attempt < m_maxRecoveryRetries
        && !m_recoveringAfterDownloadError;

    if (canRecover) {
        PendingDownload retry = m_activeDownload;
        retry.attempt += 1;
        m_downloadQueue.prepend(retry);
        m_downloadActive = false;
        m_activeDownload = PendingDownload{};
        m_recoveringAfterDownloadError = true;
        LOGW("service") << "Will recover session and retry file:"
                        << retry.file.name
                        << "nextAttempt:" << retry.attempt
                        << "failureCount:" << failureCount;
        m_bridge->recoverDownloadSession();
        return;
    }

    emit downloadError(fileName, errorMessage);

    m_downloadActive = false;
    m_activeDownload = PendingDownload{};
    QTimer::singleShot(m_postErrorDelayMs, this, &ThetaCameraService::kickNextDownload);
}

void ThetaCameraService::resumeAfterRecovery()
{
    if (!m_recoveringAfterDownloadError) {
        return;
    }

    LOGI("service") << "Resuming queue after download recovery";
    m_recoveringAfterDownloadError = false;
    if (!m_downloadActive) {
        QTimer::singleShot(m_postErrorDelayMs, this, &ThetaCameraService::kickNextDownload);
    }
}

void ThetaCameraService::kickNextDownload()
{
    if (m_downloadActive || m_downloadQueue.isEmpty()) {
        return;
    }

    m_activeDownload = m_downloadQueue.takeFirst();
    m_downloadActive = true;
    LOGD("service") << "Starting serial download:" << m_activeDownload.file.name
                    << "destDir:" << m_activeDownload.destinationPath
                    << "attempt:" << m_activeDownload.attempt
                    << "remaining queue:" << m_downloadQueue.size();
    m_bridge->requestDownloadFile(m_activeDownload.file, m_activeDownload.destinationPath);
}
