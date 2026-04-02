#include "thetaexplorer/MainWindow.h"
#include "thetaexplorer/ThetaCameraService.h"
#include "thetaexplorer/ThumbnailGridWidget.h"
#include "thetaexplorer/PreviewPanel.h"
#include "thetaexplorer/FileMetadataPanel.h"
#include "thetaexplorer/DownloadManager.h"
#include "thetaexplorer/ConfirmDeleteDialog.h"
#include "thetaexplorer/ColorUtils.h"
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStatusBar>
#include <QDebug>
#include <QMessageBox>
#include <QScrollBar>

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

    m_folderLabel = new QLabel(this);
    m_folderLabel->setStyleSheet("color: #555555; font-size: 11px;");
    m_folderLabel->setMaximumWidth(200);

    // Update folder label to show short path
    auto updateFolderLabel = [this]() {
        QString path = m_downloadFolder;
        if (path.length() > 30)
            path = "..." + path.right(27);
        m_folderLabel->setText(path);
    };
    updateFolderLabel();
    connect(m_folderBtn, &QPushButton::clicked, this, [this, updateFolderLabel]() {
        onBrowseFolderClicked();
        updateFolderLabel();
    });

    tbLayout->addWidget(m_cameraLabel);
    tbLayout->addWidget(sep1);
    tbLayout->addWidget(m_folderBtn);
    tbLayout->addWidget(m_folderLabel);
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
                if (m_selectedFiles.size() == 1 &&
                    m_selectedFiles.first().devicePath == path) {
                    m_previewPanel->showFile(m_selectedFiles.first(), px);
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
}

void MainWindow::applyStyles()
{
    // Toolbar buttons already have inline styles set in setupUI
    // Additional styles via QSS loaded in main.cpp
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
    m_selectedFiles.clear();
    updateButtonStates();
    setStatusMessage("Camera disconnected.", ColorUtils::TEXT_DIM);
}

void MainWindow::onFileListReady(const QList<CameraFileInfo>& files)
{
    m_gridWidget->setFiles(files);
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_selectedFiles.clear();
    updateButtonStates();

    int imgs   = 0, vids = 0, raws = 0;
    for (const auto& f : files) {
        if (f.isVideo) vids++;
        else if (f.isRaw) raws++;
        else imgs++;
    }
    QString summary = QString("%1 file%2  (%3 photo%4, %5 video%6, %7 RAW)")
        .arg(files.size()).arg(files.size() != 1 ? "s" : "")
        .arg(imgs).arg(imgs != 1 ? "s" : "")
        .arg(vids).arg(vids != 1 ? "s" : "")
        .arg(raws);
    setStatusMessage(summary);
}

void MainWindow::onSelectionChanged(const QList<CameraFileInfo>& selected)
{
    m_selectedFiles = selected;
    updateButtonStates();

    if (selected.isEmpty()) {
        m_previewPanel->clearPreview();
        m_metaPanel->clearMetadata();
    } else if (selected.size() == 1) {
        const CameraFileInfo& f = selected.first();
        // Find tile's current thumbnail (if already loaded)
        // We pass a null pixmap; service will have already cached via thumbnailReady
        m_previewPanel->showFile(f, QPixmap()); // will update when thumbnailReady fires
        m_metaPanel->showMetadata(f);
        // Request thumbnail explicitly in case it's not loaded yet
        m_service->requestThumbnail(f);
    } else {
        m_previewPanel->showMultipleSelection(selected.size());
        m_metaPanel->showMultipleSelection(selected.size());
    }
}

void MainWindow::onDownloadClicked()
{
    if (m_selectedFiles.isEmpty()) return;

    // Ensure download folder exists
    QDir().mkpath(m_downloadFolder);

    m_downloadTotal = m_selectedFiles.size();
    m_downloadDone  = 0;

    m_progressBar->setRange(0, m_downloadTotal);
    m_progressBar->setValue(0);
    m_progressLabel->setText(QString("Downloading 0 / %1").arg(m_downloadTotal));
    m_progressBar->show();
    m_progressLabel->show();
    m_downloadBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);

    m_service->downloadFiles(m_selectedFiles, m_downloadFolder);
    setStatusMessage(QString("Downloading %1 file%2 to %3...")
        .arg(m_downloadTotal)
        .arg(m_downloadTotal != 1 ? "s" : "")
        .arg(m_downloadFolder));
}

void MainWindow::onDeleteClicked()
{
    if (m_selectedFiles.isEmpty()) return;
    if (!ConfirmDeleteDialog::confirm(m_selectedFiles.size(), this)) return;

    m_deleteBtn->setEnabled(false);
    m_downloadBtn->setEnabled(false);
    setStatusMessage(QString("Deleting %1 file%2 from camera...")
        .arg(m_selectedFiles.size())
        .arg(m_selectedFiles.size() != 1 ? "s" : ""),
        ColorUtils::WARNING);

    m_service->deleteFiles(m_selectedFiles);
}

void MainWindow::onBrowseFolderClicked()
{
    QString chosen = QFileDialog::getExistingDirectory(
        this, "Choose download folder",
        m_downloadFolder,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (!chosen.isEmpty()) {
        m_downloadFolder = chosen;
    }
}

void MainWindow::onDownloadProgress(const QString& fileName, int percent)
{
    Q_UNUSED(fileName)
    // Update overall bar based on individual file progress
    // Approximate: each file contributes (percent / total) to overall
    // We use per-file 0-100 to show current file progress
    m_progressBar->setValue(m_downloadDone * 100 + percent);
    m_progressBar->setRange(0, m_downloadTotal * 100);
    m_progressLabel->setText(
        QString("%1 / %2  (%3%)").arg(m_downloadDone).arg(m_downloadTotal).arg(percent)
    );
}

void MainWindow::onDownloadFileCompleted(const QString& fileName, const QString& path)
{
    m_downloadDone++;
    m_progressBar->setValue(m_downloadDone * 100);
    m_progressLabel->setText(
        QString("%1 / %2 done").arg(m_downloadDone).arg(m_downloadTotal)
    );

    if (m_downloadDone >= m_downloadTotal) {
        m_progressBar->hide();
        m_progressLabel->hide();
        m_progressBar->setRange(0, 100);
        updateButtonStates();
        setStatusMessage(
            QString("Downloaded %1 file%2 to %3")
                .arg(m_downloadDone)
                .arg(m_downloadDone != 1 ? "s" : "")
                .arg(m_downloadFolder),
            ColorUtils::SUCCESS
        );
    }
    qDebug() << "Downloaded:" << fileName << "->" << path;
}

void MainWindow::onDownloadError(const QString& fileName, const QString& error)
{
    m_downloadDone++;
    setStatusMessage("Download error: " + error, ColorUtils::ERROR_COLOR);
    if (m_downloadDone >= m_downloadTotal) {
        m_progressBar->hide();
        m_progressLabel->hide();
        updateButtonStates();
    }
}

void MainWindow::onDeleteCompleted(const QStringList& deletedPaths)
{
    m_gridWidget->removeFiles(deletedPaths);
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_selectedFiles.clear();
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
    qWarning() << "[MainWindow] Error:" << message;
    setStatusMessage(message, ColorUtils::ERROR_COLOR);
    updateButtonStates();
    m_progressBar->hide();
    m_progressLabel->hide();
}

void MainWindow::updateButtonStates()
{
    bool hasCamera   = m_service->isCameraConnected();
    bool hasSelected = !m_selectedFiles.isEmpty();
    m_downloadBtn->setEnabled(hasCamera && hasSelected);
    m_deleteBtn->setEnabled(hasCamera && hasSelected);
}

void MainWindow::setStatusMessage(const QString& msg, const QString& color)
{
    QString c = color.isEmpty() ? ColorUtils::TEXT_DIM : color;
    m_statusLabel->setStyleSheet(
        QString("padding: 0 8px; color: %1; font-size: 11px;").arg(c)
    );
    m_statusLabel->setText(msg);
}
