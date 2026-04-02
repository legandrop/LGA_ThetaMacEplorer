#pragma once
#include <QObject>
#include <QHash>
#include <QString>
#include <QTimer>
#include "thetaexplorer/CameraFileInfo.h"

class ThetaBridge;

// High-level Qt camera service - manages ThetaBridge and adds thumbnail throttling.
class ThetaCameraService : public QObject
{
    Q_OBJECT
public:
    explicit ThetaCameraService(QObject* parent = nullptr);
    ~ThetaCameraService();

    void start();
    void stop();
    void refresh();

    const QList<CameraFileInfo>& fileList() const { return m_files; }
    bool isCameraConnected() const { return m_connected; }
    bool isDownloadActive() const { return m_downloadActive; }

    void requestThumbnail(const CameraFileInfo& file);
    void downloadFiles(const QList<CameraFileInfo>& files, const QString& destinationPath);
    void deleteFiles(const QList<CameraFileInfo>& files);

signals:
    void cameraConnected(const QString& name);
    void cameraDisconnected();
    void batteryLevelChanged(int percent, bool available);
    void fileListReady(const QList<CameraFileInfo>& files);
    void thumbnailReady(const QString& devicePath, const QPixmap& thumbnail);
    void downloadProgress(const QString& fileName, int percent);
    void downloadFileCompleted(const QString& fileName, const QString& savedPath);
    void downloadError(const QString& fileName, const QString& errorMessage);
    void deleteCompleted(const QStringList& deletedPaths);
    void errorOccurred(const QString& message);

private slots:
    void onFileListUpdated(const QList<CameraFileInfo>& files);
    void drainThumbnailQueue();
    void onBridgeDownloadProgress(const QString& fileName, int percent);
    void onBridgeDownloadFileCompleted(const QString& fileName, const QString& savedPath);
    void onBridgeDownloadError(const QString& fileName, const QString& errorMessage);
    void resumeAfterRecovery();
    void kickNextDownload();

private:
    struct PendingDownload {
        CameraFileInfo file;
        QString destinationPath;
        int attempt = 1;
    };

    ThetaBridge*          m_bridge       = nullptr;
    QList<CameraFileInfo> m_files;
    QList<CameraFileInfo> m_thumbnailQueue;
    QList<PendingDownload> m_downloadQueue;
    PendingDownload        m_activeDownload;
    QTimer*               m_thumbnailTimer = nullptr;
    bool                  m_connected    = false;
    bool                  m_downloadActive = false;
    bool                  m_recoveringAfterDownloadError = false;
    int                   m_maxRecoveryRetries = 2;
    int                   m_interDownloadDelayMs = 180;
    int                   m_postErrorDelayMs = 900;
    QHash<QString, int>   m_failureCountByDevicePath;
};
