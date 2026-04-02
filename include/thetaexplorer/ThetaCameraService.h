#pragma once
#include <QObject>
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

    const QList<CameraFileInfo>& fileList() const { return m_files; }
    bool isCameraConnected() const { return m_connected; }

    void requestThumbnail(const CameraFileInfo& file);
    void downloadFiles(const QList<CameraFileInfo>& files, const QString& destinationPath);
    void deleteFiles(const QList<CameraFileInfo>& files);

signals:
    void cameraConnected(const QString& name);
    void cameraDisconnected();
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

private:
    ThetaBridge*          m_bridge       = nullptr;
    QList<CameraFileInfo> m_files;
    QList<CameraFileInfo> m_thumbnailQueue;
    QTimer*               m_thumbnailTimer = nullptr;
    bool                  m_connected    = false;
};
