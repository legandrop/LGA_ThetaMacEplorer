#include "thetaexplorer/ThetaCameraService.h"
#include "thetaexplorer/ThetaBridge.h"
#include <QTimer>
#include <QDebug>

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
                emit cameraDisconnected();
            });

    connect(m_bridge, &ThetaBridge::fileListUpdated,
            this, &ThetaCameraService::onFileListUpdated);

    connect(m_bridge, &ThetaBridge::thumbnailReady,
            this, &ThetaCameraService::thumbnailReady);

    connect(m_bridge, &ThetaBridge::downloadProgress,
            this, &ThetaCameraService::downloadProgress);

    connect(m_bridge, &ThetaBridge::downloadFileCompleted,
            this, &ThetaCameraService::downloadFileCompleted);

    connect(m_bridge, &ThetaBridge::downloadError,
            this, &ThetaCameraService::downloadError);

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
    qDebug() << "[Service] Starting camera browsing";
    m_bridge->startBrowsing();
}

void ThetaCameraService::stop()
{
    if (m_thumbnailTimer) m_thumbnailTimer->stop();
    m_bridge->stopBrowsing();
}

void ThetaCameraService::onFileListUpdated(const QList<CameraFileInfo>& files)
{
    // The bridge already debounces (300ms GCD) so this should fire only once
    // per connection burst. We always accept the new list.
    qDebug() << "[Service] fileListUpdated:" << files.size() << "files";

    m_files = files;

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
        qDebug() << "[Service] All thumbnails requested";
    }
}

void ThetaCameraService::requestThumbnail(const CameraFileInfo& file)
{
    m_bridge->requestThumbnail(file);
}

void ThetaCameraService::downloadFiles(const QList<CameraFileInfo>& files,
                                        const QString& destinationPath)
{
    m_bridge->requestDownloadFiles(files, destinationPath);
}

void ThetaCameraService::deleteFiles(const QList<CameraFileInfo>& files)
{
    m_bridge->requestDeleteFiles(files);
}
