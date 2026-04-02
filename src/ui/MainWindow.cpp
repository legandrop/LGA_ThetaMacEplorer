#include "thetaexplorer/MainWindow.h"
#include "thetaexplorer/ThetaCameraService.h"
#include "thetaexplorer/HdrGroupingService.h"
#include "thetaexplorer/ThumbnailGridWidget.h"
#include "thetaexplorer/PreviewPanel.h"
#include "thetaexplorer/FileMetadataPanel.h"
#include "thetaexplorer/DownloadManager.h"
#include "thetaexplorer/CameraCatalogDebugExporter.h"
#include "thetaexplorer/ConfirmDeleteDialog.h"
#include "thetaexplorer/ColorUtils.h"
#include "thetaexplorer/Logger.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStatusBar>
#include <QSettings>
#include <QFileInfo>
#include <QFileInfoList>
#include <QRegularExpression>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QScrollBar>

namespace {

QString chooseDirectoryNonNative(QWidget* parent,
                                 const QString& title,
                                 const QString& initialDir)
{
    QFileDialog dialog(parent, title, initialDir);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setModal(true);
    dialog.raise();
    dialog.activateWindow();

    if (dialog.exec() != QDialog::Accepted) {
        return QString();
    }

    const QStringList selected = dialog.selectedFiles();
    return selected.isEmpty() ? QString() : selected.first();
}

QString chooseSaveFileNonNative(QWidget* parent,
                                const QString& title,
                                const QString& initialPath,
                                const QString& filter)
{
    QFileDialog dialog(parent, title, initialPath, filter);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setModal(true);
    dialog.selectFile(initialPath);
    dialog.raise();
    dialog.activateWindow();

    if (dialog.exec() != QDialog::Accepted) {
        return QString();
    }

    const QStringList selected = dialog.selectedFiles();
    return selected.isEmpty() ? QString() : selected.first();
}

} // namespace

namespace {

QString findMatchingLocalFilePath(const QString& folderPath, const QString& expectedName)
{
    const QString exactPath = QDir(folderPath).filePath(expectedName);
    if (QFileInfo::exists(exactPath)) {
        return exactPath;
    }

    const QFileInfo expectedInfo(expectedName);
    const QString base = expectedInfo.completeBaseName();
    const QString suffix = expectedInfo.suffix();
    QDir dir(folderPath);
    const QStringList candidates = dir.entryList(
        QStringList()
            << QString("%1 *.%2").arg(base, suffix)
            << QString("%1.%2").arg(base, suffix),
        QDir::Files,
        QDir::Name
    );

    for (const QString& candidate : candidates) {
        const QFileInfo fi(candidate);
        const QString candidateBase = fi.completeBaseName();
        if (candidateBase == base || candidateBase.startsWith(base + " ")) {
            return dir.filePath(candidate);
        }
    }

    return QString();
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("ThetaMacExplorer");
    setMinimumSize(900, 600);
    resize(1200, 720);

    // Default download folder
    m_downloadFolder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                       + "/ThetaDownloads";

    // Camera service
    m_service = new ThetaCameraService(this);

    setupUI();
    setupConnections();
    applyStyles();
    loadSettings();

    // Start scanning for the camera
    m_service->start();
    setStatusMessage("Searching for camera...", ColorUtils::TEXT_DIM);
}

MainWindow::~MainWindow()
{
    m_service->stop();
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    auto* rootLayout = new QVBoxLayout(m_centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---- Toolbar ----
    m_toolbar = new QWidget(this);
    m_toolbar->setObjectName("toolbar");
    m_toolbar->setFixedHeight(52);
    m_toolbar->setStyleSheet(
        "QWidget#toolbar {"
        "  background: #1a1a1a;"
        "  border-bottom: 1px solid #222222;"
        "}"
    );

    auto* tbLayout = new QHBoxLayout(m_toolbar);
    tbLayout->setContentsMargins(14, 0, 14, 0);
    tbLayout->setSpacing(10);

    // Camera status indicator
    m_logoLabel = new QLabel(this);
    m_logoLabel->setFixedSize(24, 24);
    m_logoLabel->setPixmap(QPixmap(":/icons/thetaexplorer_logo.svg").scaled(
        24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_cameraLabel = new QLabel("⬤  No camera connected", this);
    m_cameraLabel->setStyleSheet("color: #444444; font-size: 13px;");
    m_cameraLabel->setObjectName("cameraLabel");

    // Separator
    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color: #2a2a2a;");
    sep1->setFixedWidth(1);

    // Download folder button
    m_folderBtn = new QPushButton("📁 Save to...", this);
    m_folderBtn->setObjectName("folderBtn");
    m_folderBtn->setFixedHeight(30);
    m_folderBtn->setStyleSheet(
        "QPushButton { background:#1d1d1d; color:#777; border:1px solid #2e2e2e; "
        "border-radius:4px; padding:0 10px; font-size:12px; }"
        "QPushButton:hover { background:#252525; color:#999; border-color:#444; }"
    );

    m_refreshBtn = new QPushButton("↻ Refresh", this);
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setFixedHeight(30);
    m_refreshBtn->setStyleSheet(
        "QPushButton { background:#1d1d1d; color:#777; border:1px solid #2e2e2e; "
        "border-radius:4px; padding:0 10px; font-size:12px; }"
        "QPushButton:hover { background:#252525; color:#999; border-color:#444; }"
    );

    m_exportCatalogBtn = new QPushButton("Export catalog", this);
    m_exportCatalogBtn->setObjectName("exportCatalogBtn");
    m_exportCatalogBtn->setFixedHeight(30);
    m_exportCatalogBtn->setEnabled(false);
    m_exportCatalogBtn->setStyleSheet(
        "QPushButton { background:#1d1d1d; color:#777; border:1px solid #2e2e2e; "
        "border-radius:4px; padding:0 10px; font-size:12px; }"
        "QPushButton:hover { background:#252525; color:#999; border-color:#444; }"
    );

    m_folderLabel = new QLabel(this);
    m_folderLabel->setStyleSheet("color: #555555; font-size: 11px;");
    m_folderLabel->setMaximumWidth(200);

    updateFolderLabel();
    connect(m_folderBtn, &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);

    tbLayout->addWidget(m_logoLabel);
    tbLayout->addWidget(m_cameraLabel);
    tbLayout->addWidget(sep1);
    tbLayout->addWidget(m_folderBtn);
    tbLayout->addWidget(m_refreshBtn);
    tbLayout->addWidget(m_folderLabel);
    tbLayout->addWidget(m_exportCatalogBtn);
    tbLayout->addStretch();

    // Progress area (hidden by default)
    m_progressLabel = new QLabel(this);
    m_progressLabel->setStyleSheet("color: #555555; font-size: 11px;");
    m_progressLabel->hide();

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setFixedSize(120, 8);
    m_progressBar->hide();

    tbLayout->addWidget(m_progressLabel);
    tbLayout->addWidget(m_progressBar);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setStyleSheet("color: #2a2a2a;");
    sep2->setFixedWidth(1);
    tbLayout->addWidget(sep2);

    // Action buttons
    m_downloadBtn = new QPushButton("↓ Download", this);
    m_downloadBtn->setObjectName("downloadBtn");
    m_downloadBtn->setFixedHeight(30);
    m_downloadBtn->setEnabled(false);

    m_deleteBtn = new QPushButton("✕ Delete", this);
    m_deleteBtn->setObjectName("deleteBtn");
    m_deleteBtn->setFixedHeight(30);
    m_deleteBtn->setEnabled(false);

    tbLayout->addWidget(m_downloadBtn);
    tbLayout->addWidget(m_deleteBtn);

    rootLayout->addWidget(m_toolbar);

    // ---- Main content area ----
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName("mainSplitter");
    m_mainSplitter->setHandleWidth(1);

    // Left: thumbnail grid
    m_gridWidget = new ThumbnailGridWidget(this);
    m_mainSplitter->addWidget(m_gridWidget);

    // Right: preview + metadata stacked vertically
    m_rightSplitter = new QSplitter(Qt::Vertical, this);
    m_rightSplitter->setHandleWidth(1);

    m_previewPanel = new PreviewPanel(this);
    m_metaPanel    = new FileMetadataPanel(this);

    m_rightSplitter->addWidget(m_previewPanel);
    m_rightSplitter->addWidget(m_metaPanel);
    m_rightSplitter->setStretchFactor(0, 3);
    m_rightSplitter->setStretchFactor(1, 1);
    m_rightSplitter->setSizes({400, 160});

    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setSizes({820, 350});

    rootLayout->addWidget(m_mainSplitter, 1);

    // Status bar
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("padding: 0 8px; color: #444444; font-size: 11px;");
    statusBar()->addPermanentWidget(m_statusLabel, 1);
    statusBar()->setStyleSheet(
        "QStatusBar { background:#111111; border-top:1px solid #1e1e1e; }"
    );
}

void MainWindow::setupConnections()
{
    connect(m_service, &ThetaCameraService::cameraConnected,
            this, &MainWindow::onCameraConnected);
    connect(m_service, &ThetaCameraService::cameraDisconnected,
            this, &MainWindow::onCameraDisconnected);
    connect(m_service, &ThetaCameraService::fileListReady,
            this, &MainWindow::onFileListReady);
    connect(m_service, &ThetaCameraService::thumbnailReady,
            m_gridWidget, &ThumbnailGridWidget::setThumbnail);
    connect(m_service, &ThetaCameraService::thumbnailReady,
            this, [this](const QString& path, const QPixmap& px) {
                // Update preview if the selected file just got its thumbnail
                if (m_selectedGroups.size() == 1 &&
                    m_selectedGroups.first().representativeDevicePath() == path) {
                    m_previewPanel->showGroup(m_selectedGroups.first(), px);
                }
            });
    connect(m_service, &ThetaCameraService::downloadProgress,
            this, &MainWindow::onDownloadProgress);
    connect(m_service, &ThetaCameraService::downloadFileCompleted,
            this, &MainWindow::onDownloadFileCompleted);
    connect(m_service, &ThetaCameraService::downloadError,
            this, &MainWindow::onDownloadError);
    connect(m_service, &ThetaCameraService::deleteCompleted,
            this, &MainWindow::onDeleteCompleted);
    connect(m_service, &ThetaCameraService::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    connect(m_gridWidget, &ThumbnailGridWidget::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(m_downloadBtn, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_deleteBtn,   &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(m_refreshBtn,  &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_exportCatalogBtn, &QPushButton::clicked, this, &MainWindow::onExportCatalogClicked);
}

void MainWindow::applyStyles()
{
    // Toolbar buttons already have inline styles set in setupUI
    // Additional styles via QSS loaded in main.cpp
}

void MainWindow::loadSettings()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());

    const QByteArray geometry = settings.value("window/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    const QByteArray mainState = settings.value("window/mainSplitter").toByteArray();
    if (!mainState.isEmpty()) {
        m_mainSplitter->restoreState(mainState);
    }

    const QByteArray rightState = settings.value("window/rightSplitter").toByteArray();
    if (!rightState.isEmpty()) {
        m_rightSplitter->restoreState(rightState);
    }

    const QString savedFolder = settings.value("downloads/folder").toString();
    if (!savedFolder.isEmpty()) {
        m_downloadFolder = savedFolder;
        updateFolderLabel();
    }
}

void MainWindow::saveSettings() const
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/mainSplitter", m_mainSplitter->saveState());
    settings.setValue("window/rightSplitter", m_rightSplitter->saveState());
    settings.setValue("downloads/folder", m_downloadFolder);
}

void MainWindow::updateFolderLabel()
{
    QString path = m_downloadFolder;
    if (path.length() > 30) {
        path = "..." + path.right(27);
    }
    m_folderLabel->setText(path);
}

QString MainWindow::groupDownloadFolderName(const MediaAssetGroup& group) const
{
    QString base = group.displayTitle;
    base.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
    base.remove(QRegularExpression("^_+|_+$"));
    if (base.isEmpty()) {
        base = "item";
    }

    QString suffix;
    if (group.isVideo) suffix = "_video";
    else if (group.isRaw) suffix = "_dng";
    else suffix = "_jpg";

    return base + suffix;
}

QString MainWindow::groupDownloadFolderPath(const MediaAssetGroup& group) const
{
    return QDir(m_downloadFolder).filePath(groupDownloadFolderName(group));
}

QList<CameraFileInfo> MainWindow::selectedFilesFlattened() const
{
    QList<CameraFileInfo> files;
    for (const MediaAssetGroup& group : m_selectedGroups) {
        files.append(group.files);
    }
    return files;
}

void MainWindow::refreshDownloadedStatus()
{
    for (MediaAssetGroup& group : m_catalogGroups) {
        updateGroupLocalStatus(group);
    }

    m_gridWidget->setGroups(m_catalogGroups);
}

void MainWindow::updateGroupLocalStatus(MediaAssetGroup& group) const
{
    const QString folderPath = groupDownloadFolderPath(group);
    int found = 0;
    int exactSizeMatches = 0;

    for (const CameraFileInfo& file : group.files) {
        const QString matchedPath = findMatchingLocalFilePath(folderPath, file.name);
        if (matchedPath.isEmpty()) {
            continue;
        }
        ++found;

        const QFileInfo localInfo(matchedPath);
        if (file.sizeBytes > 0 && localInfo.exists() && localInfo.size() == file.sizeBytes) {
            ++exactSizeMatches;
        }
    }

    group.downloadedFileCount = found;
    group.allFilesDownloaded = !group.files.isEmpty()
        && found == group.files.size()
        && exactSizeMatches == group.files.size();
    group.hasPartialLocalContent = !group.allFilesDownloaded
        && (found > 0 || QFileInfo::exists(folderPath));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::onCameraConnected(const QString& name)
{
    m_cameraLabel->setText("⬤  " + name);
    m_cameraLabel->setStyleSheet("color: #4caf7d; font-size: 13px;");
    setStatusMessage("Camera connected. Loading files...", ColorUtils::SUCCESS);
    updateButtonStates();
}

void MainWindow::onCameraDisconnected()
{
    m_cameraLabel->setText("⬤  No camera connected");
    m_cameraLabel->setStyleSheet("color: #444444; font-size: 13px;");
    m_gridWidget->clearAll();
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_catalogFiles.clear();
    m_catalogGroups.clear();
    m_selectedGroups.clear();
    updateButtonStates();
    setStatusMessage("Camera disconnected.", ColorUtils::TEXT_DIM);
}

void MainWindow::onFileListReady(const QList<CameraFileInfo>& files)
{
    m_catalogFiles = files;
    m_catalogGroups = HdrGroupingService::buildGroups(files);
    refreshDownloadedStatus();
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_selectedGroups.clear();
    updateButtonStates();

    int imgs   = 0, vids = 0, raws = 0;
    for (const auto& f : files) {
        if (f.isVideo) vids++;
        else if (f.isRaw) raws++;
        else imgs++;
    }
    int hdrJpg = 0, hdrRaw = 0;
    for (const auto& g : m_catalogGroups) {
        if (g.kind == MediaAssetKind::HdrJpegSet) hdrJpg++;
        if (g.kind == MediaAssetKind::HdrRawSet) hdrRaw++;
    }
    QString summary = QString("%1 browser item%2  (%3 photo%4, %5 video%6, %7 RAW, %8 HDR JPG, %9 HDR DNG)")
        .arg(m_catalogGroups.size()).arg(m_catalogGroups.size() != 1 ? "s" : "")
        .arg(imgs).arg(imgs != 1 ? "s" : "")
        .arg(vids).arg(vids != 1 ? "s" : "")
        .arg(raws)
        .arg(hdrJpg)
        .arg(hdrRaw);
    setStatusMessage(summary);
}

void MainWindow::onSelectionChanged(const QList<MediaAssetGroup>& selected)
{
    m_selectedGroups = selected;
    updateButtonStates();

    if (selected.isEmpty()) {
        m_previewPanel->clearPreview();
        m_metaPanel->clearMetadata();
    } else if (selected.size() == 1) {
        const MediaAssetGroup& group = selected.first();
        m_previewPanel->showGroup(group, QPixmap());
        m_metaPanel->showMetadata(group);
        m_service->requestThumbnail(group.representative);
    } else {
        m_previewPanel->showMultipleSelection(selected.size());
        m_metaPanel->showMultipleSelection(selected.size());
    }
}

void MainWindow::onDownloadClicked()
{
    if (m_selectedGroups.isEmpty()) return;

    LOGI("ui") << "Download requested. selectedGroups:" << m_selectedGroups.size()
               << "downloadRoot:" << m_downloadFolder;

    // Ensure download folder exists
    QDir().mkpath(m_downloadFolder);

    QList<MediaAssetGroup> groupsToReplace;
    QList<MediaAssetGroup> groupsToSkip;
    for (MediaAssetGroup group : m_selectedGroups) {
        updateGroupLocalStatus(group);
        if (group.allFilesDownloaded || group.hasPartialLocalContent) {
            groupsToReplace.append(group);
        }
    }

    QMessageBox::StandardButton overwriteChoice = QMessageBox::NoButton;
    if (!groupsToReplace.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Existing local files");
        msgBox.setText(QString("%1 selected item%2 already exist locally, completely or partially.")
            .arg(groupsToReplace.size())
            .arg(groupsToReplace.size() != 1 ? "s" : ""));
        msgBox.setInformativeText("Replace the whole local set/file before downloading again?");
        QPushButton* replaceBtn = msgBox.addButton("Replace Existing", QMessageBox::AcceptRole);
        QPushButton* skipBtn = msgBox.addButton("Skip Existing", QMessageBox::DestructiveRole);
        msgBox.addButton(QMessageBox::Cancel);
        msgBox.exec();

        if (msgBox.clickedButton() == replaceBtn) {
            overwriteChoice = QMessageBox::Yes;
        } else if (msgBox.clickedButton() == skipBtn) {
            overwriteChoice = QMessageBox::No;
        } else {
            return;
        }
    }

    QList<CameraFileInfo> filesToDownload = selectedFilesFlattened();
    if (overwriteChoice == QMessageBox::No) {
        filesToDownload.clear();
        for (MediaAssetGroup group : m_selectedGroups) {
            updateGroupLocalStatus(group);
            if (!group.allFilesDownloaded && !group.hasPartialLocalContent) {
                filesToDownload.append(group.files);
            }
        }
        if (filesToDownload.isEmpty()) {
            setStatusMessage("Nothing to download. All selected items already exist locally.", ColorUtils::TEXT_DIM);
            return;
        }
    } else if (overwriteChoice == QMessageBox::Yes) {
        for (const MediaAssetGroup& group : groupsToReplace) {
            LOGI("ui") << "Replacing local folder before redownload:"
                       << groupDownloadFolderPath(group);
            QDir(groupDownloadFolderPath(group)).removeRecursively();
        }
    }

    m_downloadTotal = filesToDownload.size();
    m_downloadDone  = 0;
    m_downloadHasDeterminateProgress = false;
    m_downloadInProgress = true;

    m_progressBar->setRange(0, 0);
    m_progressLabel->setText(QString("Downloading %1 item%2...")
        .arg(m_downloadTotal)
        .arg(m_downloadTotal != 1 ? "s" : ""));
    m_progressBar->show();
    m_progressLabel->show();
    updateButtonStates();

    for (const MediaAssetGroup& group : m_selectedGroups) {
        const QString targetFolder = groupDownloadFolderPath(group);
        QDir().mkpath(targetFolder);
        LOGI("ui") << "Queueing group download:"
                   << group.displayTitle
                   << "files:" << group.files.size()
                   << "targetFolder:" << targetFolder;
        m_service->downloadFiles(group.files, targetFolder);
    }
    setStatusMessage(QString("Downloading %1 file%2 to %3...")
        .arg(m_downloadTotal)
        .arg(m_downloadTotal != 1 ? "s" : "")
        .arg(m_downloadFolder));
}

void MainWindow::onDeleteClicked()
{
    if (m_selectedGroups.isEmpty()) return;
    QList<CameraFileInfo> filesToDelete = selectedFilesFlattened();
    if (!ConfirmDeleteDialog::confirm(filesToDelete.size(), this)) return;

    m_deleteBtn->setEnabled(false);
    m_downloadBtn->setEnabled(false);
    setStatusMessage(QString("Deleting %1 file%2 from camera...")
        .arg(filesToDelete.size())
        .arg(filesToDelete.size() != 1 ? "s" : ""),
        ColorUtils::WARNING);

    m_service->deleteFiles(filesToDelete);
}

void MainWindow::onBrowseFolderClicked()
{
    LOGD("ui") << "Browse folder clicked. currentFolder=" << m_downloadFolder;

    const QString chosen = chooseDirectoryNonNative(
        this,
        "Choose download folder",
        m_downloadFolder
    );

    if (!chosen.isEmpty()) {
        m_downloadFolder = chosen;
        updateFolderLabel();
        saveSettings();
        refreshDownloadedStatus();
        LOGI("ui") << "Browse folder selected:" << m_downloadFolder;
    } else {
        LOGD("ui") << "Browse folder canceled by user";
    }
}

void MainWindow::onRefreshClicked()
{
    if (m_downloadInProgress || m_service->isDownloadActive()) {
        setStatusMessage("Cannot refresh while a download is in progress.", ColorUtils::WARNING);
        return;
    }

    LOGI("ui") << "Manual camera refresh requested";
    setStatusMessage("Refreshing camera catalog...", ColorUtils::TEXT_DIM);
    m_service->refresh();
}

void MainWindow::onExportCatalogClicked()
{
    LOGD("ui") << "Export catalog clicked. catalogFiles=" << m_catalogFiles.size();

    if (m_catalogFiles.isEmpty()) {
        LOGW("ui") << "Export catalog aborted: no catalog loaded";
        QMessageBox::information(this, "Export catalog", "No camera catalog is loaded yet.");
        return;
    }

    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/ThetaMacExplorer/CatalogDumps";
    if (!QDir().mkpath(defaultDir)) {
        LOGW("ui") << "Export catalog: could not ensure default dir" << defaultDir;
    }

    const QString defaultName = QString("theta-catalog-%1.json")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

    const QString outputPath = chooseSaveFileNonNative(
        this,
        "Export camera catalog",
        defaultDir + "/" + defaultName,
        "JSON files (*.json)"
    );

    if (outputPath.isEmpty()) {
        LOGD("ui") << "Export catalog canceled by user";
        return;
    }

    LOGI("ui") << "Export catalog target path:" << outputPath;

    QString errorMessage;
    if (!CameraCatalogDebugExporter::exportCatalog(m_catalogFiles, outputPath, &errorMessage)) {
        LOGW("ui") << "Export catalog failed:" << errorMessage;
        QMessageBox::warning(this, "Export catalog", errorMessage);
        setStatusMessage("Catalog export failed: " + errorMessage, ColorUtils::ERROR_COLOR);
        return;
    }

    LOGI("ui") << "Export catalog completed:" << outputPath;
    setStatusMessage(
        QString("Camera catalog exported to %1").arg(outputPath),
        ColorUtils::SUCCESS
    );
}

void MainWindow::onDownloadProgress(const QString& fileName, int percent)
{
    if (!m_downloadHasDeterminateProgress) {
        m_downloadHasDeterminateProgress = true;
        m_progressBar->setRange(0, m_downloadTotal * 100);
    }

    m_progressBar->setValue(m_downloadDone * 100 + percent);
    m_progressLabel->setText(
        QString("%1 / %2  %3%  %4")
            .arg(m_downloadDone)
            .arg(m_downloadTotal)
            .arg(percent)
            .arg(fileName)
    );
}

void MainWindow::onDownloadFileCompleted(const QString& fileName, const QString& path)
{
    m_downloadDone++;
    if (m_downloadHasDeterminateProgress) {
        m_progressBar->setValue(m_downloadDone * 100);
    }
    m_progressLabel->setText(
        QString("%1 / %2 done  %3").arg(m_downloadDone).arg(m_downloadTotal).arg(fileName)
    );

    if (m_downloadDone >= m_downloadTotal) {
        m_downloadInProgress = false;
        m_progressBar->hide();
        m_progressLabel->hide();
        m_progressBar->setRange(0, 100);
        m_downloadHasDeterminateProgress = false;
        refreshDownloadedStatus();
        updateButtonStates();
        setStatusMessage(
            QString("Downloaded %1 file%2 to %3")
                .arg(m_downloadDone)
                .arg(m_downloadDone != 1 ? "s" : "")
                .arg(m_downloadFolder),
            ColorUtils::SUCCESS
        );
    }
    LOGD("ui") << "Downloaded:" << fileName << "->" << path;
}

void MainWindow::onDownloadError(const QString& fileName, const QString& error)
{
    m_downloadDone++;
    LOGW("ui") << "Download error surfaced to UI:"
               << fileName
               << "completed:" << m_downloadDone
               << "of" << m_downloadTotal
               << "message:" << error;
    setStatusMessage("Download error: " + fileName + " - " + error, ColorUtils::ERROR_COLOR);
    if (m_downloadDone >= m_downloadTotal) {
        m_downloadInProgress = false;
        m_progressBar->hide();
        m_progressLabel->hide();
        m_progressBar->setRange(0, 100);
        m_downloadHasDeterminateProgress = false;
        refreshDownloadedStatus();
        updateButtonStates();
    }
}

void MainWindow::onDeleteCompleted(const QStringList& deletedPaths)
{
    m_gridWidget->removeFiles(deletedPaths);
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_selectedGroups.clear();
    updateButtonStates();
    setStatusMessage(
        QString("Deleted %1 file%2 from camera.")
            .arg(deletedPaths.size())
            .arg(deletedPaths.size() != 1 ? "s" : ""),
        ColorUtils::SUCCESS
    );
}

void MainWindow::onErrorOccurred(const QString& message)
{
    LOGW("ui") << "Error:" << message;
    setStatusMessage(message, ColorUtils::ERROR_COLOR);
    m_downloadInProgress = false;
    updateButtonStates();
    m_progressBar->hide();
    m_progressLabel->hide();
}

void MainWindow::updateButtonStates()
{
    bool hasCamera   = m_service->isCameraConnected();
    bool hasSelected = !m_selectedGroups.isEmpty();
    bool hasCatalog  = !m_catalogFiles.isEmpty();
    const bool busy = m_downloadInProgress || m_service->isDownloadActive();
    m_downloadBtn->setEnabled(hasCamera && hasSelected && !busy);
    m_deleteBtn->setEnabled(hasCamera && hasSelected && !busy);
    m_folderBtn->setEnabled(!busy);
    m_refreshBtn->setEnabled(hasCamera && !busy);
    m_exportCatalogBtn->setEnabled(hasCamera && hasCatalog);
}

void MainWindow::setStatusMessage(const QString& msg, const QString& color)
{
    QString c = color.isEmpty() ? ColorUtils::TEXT_DIM : color;
    m_statusLabel->setStyleSheet(
        QString("padding: 0 8px; color: %1; font-size: 11px;").arg(c)
    );
    m_statusLabel->setText(msg);
}
