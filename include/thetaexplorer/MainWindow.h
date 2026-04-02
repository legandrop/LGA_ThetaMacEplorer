#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include "thetaexplorer/CameraFileInfo.h"
#include "thetaexplorer/MediaAssetGroup.h"

class ThetaCameraService;
class ThumbnailGridWidget;
class PreviewPanel;
class FileMetadataPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onCameraConnected(const QString& name);
    void onCameraDisconnected();
    void onFileListReady(const QList<CameraFileInfo>& files);
    void onSelectionChanged(const QList<MediaAssetGroup>& selected);
    void onDownloadClicked();
    void onDeleteClicked();
    void onBrowseFolderClicked();
    void onExportCatalogClicked();
    void onDownloadProgress(const QString& fileName, int percent);
    void onDownloadFileCompleted(const QString& fileName, const QString& path);
    void onDownloadError(const QString& fileName, const QString& error);
    void onDeleteCompleted(const QStringList& deletedPaths);
    void onErrorOccurred(const QString& message);

private:
    void setupUI();
    void setupConnections();
    void applyStyles();
    void loadSettings();
    void saveSettings() const;
    void updateFolderLabel();
    void refreshDownloadedStatus();
    QString groupDownloadFolderName(const MediaAssetGroup& group) const;
    QString groupDownloadFolderPath(const MediaAssetGroup& group) const;
    QList<CameraFileInfo> selectedFilesFlattened() const;
    void updateButtonStates();
    void setStatusMessage(const QString& msg, const QString& color = "#b2b2b2");

protected:
    void closeEvent(QCloseEvent* event) override;

    // Service
    ThetaCameraService*  m_service        = nullptr;

    // Main layout widgets
    QWidget*             m_centralWidget  = nullptr;
    QSplitter*           m_mainSplitter   = nullptr;
    QSplitter*           m_rightSplitter  = nullptr;
    ThumbnailGridWidget* m_gridWidget     = nullptr;
    PreviewPanel*        m_previewPanel   = nullptr;
    FileMetadataPanel*   m_metaPanel      = nullptr;

    // Toolbar
    QWidget*             m_toolbar        = nullptr;
    QLabel*              m_logoLabel      = nullptr;
    QLabel*              m_cameraLabel    = nullptr;
    QPushButton*         m_folderBtn      = nullptr;
    QPushButton*         m_exportCatalogBtn = nullptr;
    QLabel*              m_folderLabel    = nullptr;
    QPushButton*         m_downloadBtn    = nullptr;
    QPushButton*         m_deleteBtn      = nullptr;
    QProgressBar*        m_progressBar    = nullptr;
    QLabel*              m_progressLabel  = nullptr;

    // Status bar label
    QLabel*              m_statusLabel    = nullptr;

    // State
    QString              m_downloadFolder;
    QList<CameraFileInfo> m_catalogFiles;
    QList<MediaAssetGroup> m_catalogGroups;
    QList<MediaAssetGroup> m_selectedGroups;
    int                  m_downloadTotal  = 0;
    int                  m_downloadDone   = 0;
};
