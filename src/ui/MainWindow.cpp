#include "thetaexplorer/MainWindow.h"
#include "thetaexplorer/ThetaCameraService.h"
#include "thetaexplorer/HdrGroupingService.h"
#include "thetaexplorer/ThumbnailGridWidget.h"
#include "thetaexplorer/PreviewPanel.h"
#include "thetaexplorer/FileMetadataPanel.h"
#include "thetaexplorer/DownloadManager.h"
#include "thetaexplorer/ConfirmDialog.h"
#include "thetaexplorer/HelpDialog.h"
#include "thetaexplorer/StatusWidgets.h"
#include "thetaexplorer/AppPaths.h"
#include "thetaexplorer/ColorUtils.h"
#include "thetaexplorer/UiIcons.h"
#include "thetaexplorer/Logger.h"
#include "thetaexplorer/popover/MessagePopover.h"
#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QLabel>
#include <QMenuBar>
#include <QPushButton>
#include <QProgressBar>
#include <QStatusBar>
#include <QStyle>
#include <QSettings>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
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


namespace {

// Colores de los glyphs de la toolbar (normal / deshabilitado).
constexpr auto kIconNeutral         = "#b2b2b2";
constexpr auto kIconNeutralDisabled = "#4a4a4a";
constexpr auto kIconPrimary         = "#d6cbf5";
constexpr auto kIconPrimaryDisabled = "#3a3a50";
constexpr auto kIconDanger          = "#e7a3ae";
constexpr auto kIconDangerDisabled  = "#40282e";
constexpr int  kToolbarIconSize     = 15;
constexpr int  kFolderLabelWidth    = 230;
constexpr int  kLowBatteryPercent   = 20;

QFrame* toolbarSeparator(QWidget* parent)
{
    auto* sep = new QFrame(parent);
    sep->setObjectName("toolbarSeparator");
    sep->setFixedSize(1, 22);
    return sep;
}

QPushButton* toolbarButton(const QString& text, const char* objectName, QWidget* parent)
{
    auto* btn = new QPushButton(text, parent);
    btn->setObjectName(objectName);
    btn->setFixedHeight(30);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setIconSize(QSize(kToolbarIconSize, kToolbarIconSize));
    return btn;
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
    setupMenus();
    setupConnections();
    applyButtonIcons();
    loadSettings();

    // Start scanning for the camera
    m_service->start();
    onCameraDisconnected();
    // La toolbar y la grilla ya dicen que se esta buscando la camara.
    setStatusMessage(QString());
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
    // Izquierda: estado de la camara y bateria. Despues: destino de las descargas.
    // Derecha: progreso, acciones (Delete separado de Download) y el boton de Help.
    m_toolbar = new QWidget(this);
    m_toolbar->setObjectName("toolbar");
    m_toolbar->setAttribute(Qt::WA_StyledBackground);
    m_toolbar->setFixedHeight(52);

    auto* tbLayout = new QHBoxLayout(m_toolbar);
    tbLayout->setContentsMargins(14, 0, 14, 0);
    tbLayout->setSpacing(10);

    m_cameraDot = new StatusDot(this);
    m_cameraLabel = new QLabel(this);
    m_cameraLabel->setObjectName("cameraLabel");

    m_batteryBox = new QWidget(this);
    m_batteryBox->setObjectName("batteryBox");
    auto* batteryLayout = new QHBoxLayout(m_batteryBox);
    batteryLayout->setContentsMargins(0, 0, 0, 0);
    batteryLayout->setSpacing(5);
    m_batteryIcon = new QLabel(m_batteryBox);
    m_batteryLabel = new QLabel(m_batteryBox);
    m_batteryLabel->setObjectName("batteryLabel");
    batteryLayout->addWidget(m_batteryIcon);
    batteryLayout->addWidget(m_batteryLabel);
    m_batteryBox->hide();

    m_folderBtn = toolbarButton("Save to…", "toolbarButton", this);
    m_folderLabel = new QLabel(this);
    m_folderLabel->setObjectName("folderLabel");
    m_folderLabel->setFixedWidth(kFolderLabelWidth);
    updateFolderLabel();

    m_refreshBtn = toolbarButton(QString(), "iconButton", this);
    m_refreshBtn->setFixedWidth(30);
    m_refreshBtn->setAccessibleName("Refresh");

    m_progressLabel = new QLabel(this);
    m_progressLabel->setObjectName("progressLabel");
    m_progressLabel->hide();
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setFixedSize(120, 8);
    m_progressBar->setTextVisible(false);
    m_progressBar->hide();

    m_downloadBtn = toolbarButton("Download", "downloadBtn", this);
    m_downloadBtn->setEnabled(false);
    m_deleteBtn = toolbarButton("Delete", "deleteBtn", this);
    m_deleteBtn->setEnabled(false);

    m_helpBtn = toolbarButton(QString(), "helpBtn", this);
    m_helpBtn->setFixedWidth(30);
    m_helpBtn->setAccessibleName("Help");

    tbLayout->addWidget(m_cameraDot, 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_cameraLabel, 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_batteryBox, 0, Qt::AlignVCenter);
    tbLayout->addWidget(toolbarSeparator(this), 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_folderBtn, 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_folderLabel, 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_refreshBtn, 0, Qt::AlignVCenter);
    tbLayout->addStretch(1);
    tbLayout->addWidget(m_progressLabel, 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_progressBar, 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_downloadBtn, 0, Qt::AlignVCenter);
    tbLayout->addSpacing(14);
    tbLayout->addWidget(m_deleteBtn, 0, Qt::AlignVCenter);
    tbLayout->addWidget(toolbarSeparator(this), 0, Qt::AlignVCenter);
    tbLayout->addWidget(m_helpBtn, 0, Qt::AlignVCenter);

    rootLayout->addWidget(m_toolbar);

    // ---- Main content area ----
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setObjectName("mainSplitter");
    m_mainSplitter->setHandleWidth(1);

    // Left: grilla de thumbnails, o el estado vacio cuando no hay nada que mostrar
    m_gridStack = new QStackedWidget(this);
    m_gridWidget = new ThumbnailGridWidget(this);
    m_emptyState = new EmptyStateWidget(this);
    m_gridStack->addWidget(m_gridWidget);   // 0
    m_gridStack->addWidget(m_emptyState);   // 1
    m_mainSplitter->addWidget(m_gridStack);

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
    m_statusLabel->setObjectName("statusLabel");
    statusBar()->addPermanentWidget(m_statusLabel, 1);
    statusBar()->setSizeGripEnabled(false);
}

void MainWindow::setupMenus()
{
    // En macOS la barra de menu es la nativa: el About con AboutRole lo mueve Qt al menu de
    // la app ("About ThetaMacExplorer"), y el Help queda con Cmd+?.
    QMenu* helpMenu = menuBar()->addMenu("Help");

    auto* helpAction = new QAction("ThetaMacExplorer Help", this);
    helpAction->setShortcut(QKeySequence::HelpContents);
    connect(helpAction, &QAction::triggered, this, &MainWindow::onHelpRequested);
    helpMenu->addAction(helpAction);

    auto* aboutAction = new QAction("About ThetaMacExplorer", this);
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onHelpRequested);
    helpMenu->addAction(aboutAction);
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
    connect(m_service, &ThetaCameraService::batteryLevelChanged,
            this, &MainWindow::onBatteryLevelChanged);
    connect(m_service, &ThetaCameraService::deleteCompleted,
            this, &MainWindow::onDeleteCompleted);
    connect(m_service, &ThetaCameraService::errorOccurred,
            this, &MainWindow::onErrorOccurred);

    connect(m_gridWidget, &ThumbnailGridWidget::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(m_folderBtn,   &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);
    connect(m_downloadBtn, &QPushButton::clicked, this, &MainWindow::onDownloadClicked);
    connect(m_deleteBtn,   &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(m_refreshBtn,  &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_helpBtn,     &QPushButton::clicked, this, &MainWindow::onHelpRequested);
}

void MainWindow::applyButtonIcons()
{
    const qreal dpr = devicePixelRatioF();
    m_folderBtn->setIcon(UiIcons::buttonIcon("folder", kToolbarIconSize,
        QColor(kIconNeutral), QColor(kIconNeutralDisabled), dpr));
    m_refreshBtn->setIcon(UiIcons::buttonIcon("refresh", kToolbarIconSize,
        QColor(kIconNeutral), QColor(kIconNeutralDisabled), dpr));
    m_downloadBtn->setIcon(UiIcons::buttonIcon("download", kToolbarIconSize,
        QColor(kIconPrimary), QColor(kIconPrimaryDisabled), dpr));
    m_deleteBtn->setIcon(UiIcons::buttonIcon("trash", kToolbarIconSize,
        QColor(kIconDanger), QColor(kIconDangerDisabled), dpr));
    m_helpBtn->setIcon(UiIcons::buttonIcon("help", 16,
        QColor(ColorUtils::TXT_SECUNDARIO), QColor(kIconNeutralDisabled), dpr));
    m_helpBtn->setIconSize(QSize(16, 16));
    m_batteryIcon->setPixmap(UiIcons::pixmap("battery", 18, QColor(ColorUtils::TXT_SECUNDARIO), dpr));
}

void MainWindow::loadSettings()
{
    QSettings settings(AppPaths::settingsFile(), QSettings::IniFormat);

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
    QSettings settings(AppPaths::settingsFile(), QSettings::IniFormat);
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/mainSplitter", m_mainSplitter->saveState());
    settings.setValue("window/rightSplitter", m_rightSplitter->saveState());
    settings.setValue("downloads/folder", m_downloadFolder);
}

void MainWindow::updateFolderLabel()
{
    // Recortada por el principio: lo que importa es la carpeta final.
    QString path = m_downloadFolder;
    const QString home = QDir::homePath();
    if (path.startsWith(home)) {
        path = "~" + path.mid(home.size());
    }
    m_folderLabel->ensurePolished();   // la fuente de 12 px sale del QSS
    const QFontMetrics fm(m_folderLabel->font());
    m_folderLabel->setText(fm.elidedText(path, Qt::ElideLeft, kFolderLabelWidth));
}

void MainWindow::updateGridPage()
{
    const bool hasCamera = m_service->isCameraConnected();
    if (!hasCamera) {
        m_emptyState->setContent("Connect your RICOH THETA",
            "Plug the camera in with a USB cable and turn it on. "
            "Its photos and videos will show up here.", true);
        m_gridStack->setCurrentIndex(1);
    } else if (m_catalogLoaded && m_catalogGroups.isEmpty()) {
        m_emptyState->setContent("The camera is empty",
            "Photos and videos you take will show up here.", false);
        m_gridStack->setCurrentIndex(1);
    } else {
        m_gridStack->setCurrentIndex(0);
    }
}

void MainWindow::onHelpRequested()
{
    HelpDialog dlg(m_downloadFolder, this);
    dlg.exec();
}

QString MainWindow::groupDownloadFolderName(const MediaAssetGroup& group) const
{
    const QString datePart = group.captureTime.isValid()
        ? group.captureTime.toString("yyMMdd")
        : QStringLiteral("000000");

    QString base = group.displayTitle;
    base.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
    base.remove(QRegularExpression("^_+|_+$"));
    if (base.isEmpty()) {
        base = "item";
    }

    if (group.isVideo) {
        return QStringLiteral("video_%1_%2_").arg(datePart, base);
    }
    if (group.isRaw) {
        return QStringLiteral("HDRI_%1_%2_dng").arg(datePart, base);
    }
    return QStringLiteral("HDRI_%1_%2_jpg").arg(datePart, base);
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
    m_cameraDot->setMode(StatusDot::Mode::Connected);
    m_cameraLabel->setText(name);
    m_cameraLabel->setProperty("state", "connected");
    m_cameraLabel->style()->unpolish(m_cameraLabel);
    m_cameraLabel->style()->polish(m_cameraLabel);
    m_catalogLoaded = false;
    updateGridPage();
    setStatusMessage("Camera connected. Loading files...", ColorUtils::SUCCESS);
    updateButtonStates();
}

void MainWindow::onCameraDisconnected()
{
    m_cameraDot->setMode(StatusDot::Mode::Searching);
    m_cameraLabel->setText("Searching for camera…");
    m_cameraLabel->setProperty("state", "searching");
    m_cameraLabel->style()->unpolish(m_cameraLabel);
    m_cameraLabel->style()->polish(m_cameraLabel);
    m_batteryBox->hide();
    m_catalogLoaded = false;
    if (m_downloadInProgress) {
        LOGW("ui") << "Camera disconnected during a download:" << m_downloadDone
                   << "of" << m_downloadTotal << "files done";
    }
    m_downloadInProgress = false;
    m_downloadHasDeterminateProgress = false;
    m_progressBar->hide();
    m_progressLabel->hide();
    m_progressBar->setRange(0, 100);
    m_gridWidget->clearAll();
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_catalogFiles.clear();
    m_catalogGroups.clear();
    m_selectedGroups.clear();
    updateGridPage();
    updateButtonStates();
    setStatusMessage("No camera connected.");
}

void MainWindow::onFileListReady(const QList<CameraFileInfo>& files)
{
    m_catalogFiles = files;
    m_catalogGroups = HdrGroupingService::buildGroups(files);
    refreshDownloadedStatus();
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_selectedGroups.clear();
    m_catalogLoaded = true;
    updateGridPage();
    updateButtonStates();

    // Resumen por item (lo que se ve en la grilla), no por archivo.
    int photos = 0, raws = 0, vids = 0, hdrSets = 0;
    for (const auto& g : m_catalogGroups) {
        switch (g.kind) {
            case MediaAssetKind::SinglePhoto: photos++; break;
            case MediaAssetKind::SingleRaw:   raws++; break;
            case MediaAssetKind::Video:       vids++; break;
            case MediaAssetKind::HdrJpegSet:
            case MediaAssetKind::HdrRawSet:   hdrSets++; break;
        }
    }
    QStringList parts;
    if (hdrSets) parts << QString("%1 HDR set%2").arg(hdrSets).arg(hdrSets != 1 ? "s" : "");
    if (photos)  parts << QString("%1 photo%2").arg(photos).arg(photos != 1 ? "s" : "");
    if (raws)    parts << QString("%1 RAW").arg(raws);
    if (vids)    parts << QString("%1 video%2").arg(vids).arg(vids != 1 ? "s" : "");
    QString summary = QString("%1 item%2 on camera")
        .arg(m_catalogGroups.size()).arg(m_catalogGroups.size() != 1 ? "s" : "");
    if (!parts.isEmpty()) {
        summary += ": " + parts.join(", ") + QString(" (%1 files)").arg(files.size());
    }
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

    // Copia de la seleccion: mientras el cartel esta abierto una desconexion o un catalogo
    // nuevo vacian m_selectedGroups.
    const QList<MediaAssetGroup> selection = m_selectedGroups;

    LOGI("ui") << "Download requested. selectedGroups:" << selection.size()
               << "downloadRoot:" << m_downloadFolder;

    // Ensure download folder exists
    QDir().mkpath(m_downloadFolder);

    QList<MediaAssetGroup> groupsWithLocal;
    for (MediaAssetGroup group : selection) {
        updateGroupLocalStatus(group);
        if (group.allFilesDownloaded || group.hasPartialLocalContent) {
            groupsWithLocal.append(group);
        }
    }

    enum class OverwriteChoice { None, Replace, Skip };
    OverwriteChoice overwriteChoice = OverwriteChoice::None;
    if (!groupsWithLocal.isEmpty()) {
        const int existing = groupsWithLocal.size();
        const int total = selection.size();
        ConfirmDialog::Spec spec;
        spec.iconName = "folder_check";
        spec.title = (total == 1)
            ? QString("This item already has files in the download folder")
            : QString("%1 of %2 items already %3 files in the download folder")
                  .arg(existing).arg(total).arg(existing == 1 ? "has" : "have");
        spec.subtitle = (existing == 1)
            ? QString("Skip the files that are already there and download only what's missing, "
                      "or replace the local copy.")
            : QString("Skip the files that are already there and download only what's missing, "
                      "or replace the local copies.");
        spec.hintHtml = "Press <b>Enter</b> to skip, <b>Esc</b> to cancel.";
        spec.buttons = {
            {"Cancel (esc)", ConfirmDialog::ButtonStyle::Neutral},
            {"Replace", ConfirmDialog::ButtonStyle::Neutral},
            {"Skip Existing (enter)", ConfirmDialog::ButtonStyle::Action},
        };
        spec.defaultIndex = 2;
        spec.escapeIndex = 0;
        spec.noFocusIndexes = {1};   // Replace borra la carpeta local: solo con click

        const int choice = ConfirmDialog::ask(this, spec);
        if (choice == 1) {
            overwriteChoice = OverwriteChoice::Replace;
        } else if (choice == 2) {
            overwriteChoice = OverwriteChoice::Skip;
        } else {
            return;
        }
    }

    // La camara pudo desconectarse con el cartel abierto: no tocar nada local.
    if (!m_service->isCameraConnected()) {
        setStatusMessage("The camera was disconnected. Nothing was downloaded.", ColorUtils::WARNING);
        return;
    }

    // Que se encola por grupo. Con "Skip Existing" se saltean los ARCHIVOS que ya estan en
    // disco con el mismo tamano; un set a medio bajar se completa. (Antes de v0.994 se
    // calculaba la lista pero se encolaban todos los grupos igual.)
    QList<QPair<MediaAssetGroup, QList<CameraFileInfo>>> queue;
    int filesToDownload = 0;
    for (const MediaAssetGroup& group : selection) {
        QList<CameraFileInfo> files = group.files;
        if (overwriteChoice == OverwriteChoice::Skip) {
            files.clear();
            const QString folder = groupDownloadFolderPath(group);
            for (const CameraFileInfo& file : group.files) {
                const QString local = findMatchingLocalFilePath(folder, file.name);
                const bool complete = !local.isEmpty()
                    && (file.sizeBytes <= 0 || QFileInfo(local).size() == file.sizeBytes);
                if (!complete) {
                    files.append(file);
                }
            }
        }
        if (!files.isEmpty()) {
            filesToDownload += files.size();
            queue.append({group, files});
        }
    }

    if (filesToDownload == 0) {
        setStatusMessage("Nothing to download. All selected files are already in the download folder.");
        return;
    }

    if (overwriteChoice == OverwriteChoice::Replace) {
        for (const MediaAssetGroup& group : groupsWithLocal) {
            LOGI("ui") << "Replacing local folder before redownload:"
                       << groupDownloadFolderPath(group);
            QDir(groupDownloadFolderPath(group)).removeRecursively();
        }
    }

    m_downloadTotal = filesToDownload;
    m_downloadDone  = 0;
    m_downloadErrors = 0;
    m_lastDownloadError.clear();
    m_downloadHasDeterminateProgress = false;
    m_downloadInProgress = true;

    m_progressBar->setRange(0, 0);
    m_progressLabel->setText(QString("Downloading %1 file%2...")
        .arg(m_downloadTotal)
        .arg(m_downloadTotal != 1 ? "s" : ""));
    m_progressBar->show();
    m_progressLabel->show();
    updateButtonStates();

    for (const auto& entry : queue) {
        const QString targetFolder = groupDownloadFolderPath(entry.first);
        QDir().mkpath(targetFolder);
        LOGI("ui") << "Queueing group download:"
                   << entry.first.displayTitle
                   << "files:" << entry.second.size()
                   << "targetFolder:" << targetFolder;
        m_service->downloadFiles(entry.second, targetFolder);
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

    // Un set HDR son varios archivos: el cartel dice el total real que se borra.
    const int items = m_selectedGroups.size();
    const int files = filesToDelete.size();
    int hdrSets = 0;
    int hdrImages = 0;
    for (const MediaAssetGroup& group : m_selectedGroups) {
        if (group.isHdrSet) {
            ++hdrSets;
            hdrImages = group.imageCount();
        }
    }
    QString detail;
    if (files != items) {
        detail = QString("That's %1 files").arg(files);
        if (hdrSets == 1) {
            detail += QString(", including a %1-image HDR set").arg(hdrImages);
        } else if (hdrSets > 1) {
            detail += QString(", including %1 HDR sets").arg(hdrSets);
        }
        detail += ". ";
    }

    ConfirmDialog::Spec spec;
    spec.iconName = "trash";
    spec.title = (items == 1) ? QString("Delete this item from the camera?")
                              : QString("Delete %1 items from the camera?").arg(items);
    spec.subtitle = detail + "This can't be undone.";
    spec.hintHtml = "Press <b>Enter</b> or <b>Esc</b> to cancel. Deleting takes a click.";
    spec.buttons = {
        {"Cancel (esc)", ConfirmDialog::ButtonStyle::Neutral},
        {"Delete", ConfirmDialog::ButtonStyle::Danger},
    };
    // Enter cancela: borrar de la camara no tiene vuelta atras.
    spec.defaultIndex = 0;
    spec.escapeIndex = 0;
    spec.noFocusIndexes = {1};   // con Tab + Espacio se borraria sin ver el foco
    if (ConfirmDialog::ask(this, spec) != 1) return;

    QStringList names;
    for (const CameraFileInfo& file : filesToDelete) {
        names << file.name;
    }
    LOGI("ui") << "Delete requested for" << filesToDelete.size()
               << "file(s):" << names;

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
    setStatusMessage("Refreshing camera catalog...");
    m_service->refresh();
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
    LOGD("ui") << "Downloaded:" << fileName << "->" << path;

    if (m_downloadInProgress && m_downloadDone == m_downloadTotal) {
        finishDownloadBatch();
    }
}

void MainWindow::onDownloadError(const QString& fileName, const QString& error)
{
    m_downloadDone++;
    m_downloadErrors++;
    m_lastDownloadError = fileName + ": " + error;
    LOGW("ui") << "Download error surfaced to UI:"
               << fileName
               << "completed:" << m_downloadDone
               << "of" << m_downloadTotal
               << "message:" << error;
    setStatusMessage("Download error: " + fileName + " - " + error, ColorUtils::ERROR_COLOR);
    if (m_downloadInProgress && m_downloadDone == m_downloadTotal) {
        finishDownloadBatch();
    }
}

void MainWindow::finishDownloadBatch()
{
    m_downloadInProgress = false;
    m_progressBar->hide();
    m_progressLabel->hide();
    m_progressBar->setRange(0, 100);
    m_downloadHasDeterminateProgress = false;
    refreshDownloadedStatus();
    updateButtonStates();

    // Un solo aviso por lote, no uno por archivo.
    const int ok = m_downloadDone - m_downloadErrors;
    if (m_downloadErrors == 0) {
        const QString msg = QString("Downloaded %1 file%2 to %3")
            .arg(ok).arg(ok != 1 ? "s" : "").arg(m_downloadFolder);
        setStatusMessage(msg, ColorUtils::SUCCESS);
        notifySuccess(QString("Downloaded %1 file%2.").arg(ok).arg(ok != 1 ? "s" : ""));
    } else {
        notifyError(QString("<b>%1 of %2 files couldn't be downloaded.</b><br>%3")
            .arg(m_downloadErrors).arg(m_downloadTotal)
            .arg(m_lastDownloadError.toHtmlEscaped()));
    }
}

void MainWindow::onDeleteCompleted(const QStringList& deletedPaths)
{
    // Se saca lo borrado del catalogo y se reconstruyen los grupos: no se depende de que la
    // camara vuelva a enumerar (no siempre lo hace).
    QList<CameraFileInfo> remaining;
    for (const CameraFileInfo& file : m_catalogFiles) {
        if (!deletedPaths.contains(file.devicePath)) {
            remaining.append(file);
        }
    }
    m_catalogFiles = remaining;
    m_catalogGroups = HdrGroupingService::buildGroups(m_catalogFiles);
    refreshDownloadedStatus();
    m_previewPanel->clearPreview();
    m_metaPanel->clearMetadata();
    m_selectedGroups.clear();
    updateGridPage();
    updateButtonStates();
    const QString msg = QString("Deleted %1 file%2 from the camera.")
        .arg(deletedPaths.size())
        .arg(deletedPaths.size() != 1 ? "s" : "");
    setStatusMessage(msg, ColorUtils::SUCCESS);
    notifySuccess(msg);
}

void MainWindow::onErrorOccurred(const QString& message)
{
    LOGW("ui") << "Error:" << message;
    setStatusMessage(message, ColorUtils::ERROR_COLOR);
    notifyError(message.toHtmlEscaped());
    // Los errores de descarga llegan por onDownloadError; estos (camara, borrar) no cortan
    // un lote en curso: si la camara se va, onCameraDisconnected resetea el estado.
    updateButtonStates();
}

void MainWindow::onBatteryLevelChanged(int percent, bool available)
{
    if (!available || percent < 0) {
        m_batteryBox->hide();
        return;
    }

    const bool low = percent <= kLowBatteryPercent;
    const QColor color(low ? ColorUtils::ERROR_COLOR : ColorUtils::TXT_SECUNDARIO);
    m_batteryIcon->setPixmap(UiIcons::pixmap("battery", 18, color, devicePixelRatioF()));
    m_batteryLabel->setText(QString("%1%").arg(percent));
    m_batteryLabel->setProperty("low", low);
    m_batteryLabel->style()->unpolish(m_batteryLabel);
    m_batteryLabel->style()->polish(m_batteryLabel);
    m_batteryBox->setVisible(m_service->isCameraConnected());
}

void MainWindow::updateButtonStates()
{
    bool hasCamera   = m_service->isCameraConnected();
    bool hasSelected = !m_selectedGroups.isEmpty();
    const bool busy = m_downloadInProgress || m_service->isDownloadActive() || m_service->isDeleteActive();
    m_downloadBtn->setEnabled(hasCamera && hasSelected && !busy);
    m_deleteBtn->setEnabled(hasCamera && hasSelected && !busy);
    m_folderBtn->setEnabled(!busy);
    m_refreshBtn->setEnabled(hasCamera && !busy);
    m_downloadBtn->setText(hasSelected ? QString("Download %1").arg(m_selectedGroups.size())
                                       : QString("Download"));
}

void MainWindow::setStatusMessage(const QString& msg, const QString& color)
{
    const QString c = color.isEmpty() ? QString(ColorUtils::TXT_SECUNDARIO) : color;
    m_statusLabel->setStyleSheet(QString("QLabel#statusLabel { color: %1; }").arg(c));
    m_statusLabel->setText(msg);
}

void MainWindow::notifyError(const QString& html)
{
    // Un error nuevo reemplaza al que este abierto, para no apilar avisos.
    if (m_errorPopover) {
        m_errorPopover->closePopover();
    }
    m_errorPopover = new MessagePopover(this);
    m_errorPopover->setMessage(html);
    m_errorPopover->showPopover();   // los errores no se cierran solos
}

void MainWindow::notifySuccess(const QString& msg)
{
    auto* popover = new MessagePopover(this);
    popover->setMessage(msg.toHtmlEscaped());
    popover->showPopover(4000);
}
