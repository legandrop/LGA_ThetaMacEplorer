#include "thetaexplorer/ThumbnailGridWidget.h"
#include "thetaexplorer/ThumbnailTileWidget.h"
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>

ThumbnailGridWidget::ThumbnailGridWidget(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setObjectName("thumbnailGrid");
    setFocusPolicy(Qt::StrongFocus);

    m_container = new QWidget(this);
    m_container->setObjectName("gridContainer");
    m_container->setStyleSheet("background: #161616;");

    m_layout = new QGridLayout(m_container);
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->setSpacing(SPACING);
    m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    setWidget(m_container);
}

void ThumbnailGridWidget::setGroups(const QList<MediaAssetGroup>& groups)
{
    clearAll();
    QList<CameraFileInfo> thumbnailRequests;
    for (const MediaAssetGroup& group : groups) {
        auto* tile = new ThumbnailTileWidget(group, m_container);
        connect(tile, &ThumbnailTileWidget::clicked,
                this, &ThumbnailGridWidget::onTileClicked);
        m_tiles.append(tile);
        thumbnailRequests.append(group.representative);
    }
    relayoutGrid();
    emit thumbnailsNeeded(thumbnailRequests);
}

void ThumbnailGridWidget::setThumbnail(const QString& devicePath, const QPixmap& thumbnail)
{
    for (ThumbnailTileWidget* tile : m_tiles) {
        if (tile->group().representativeDevicePath() == devicePath) {
            tile->setThumbnail(thumbnail);
            return;
        }
    }
}

void ThumbnailGridWidget::removeFiles(const QStringList& devicePaths)
{
    QList<ThumbnailTileWidget*> toRemove;
    for (ThumbnailTileWidget* tile : m_tiles) {
        for (const CameraFileInfo& file : tile->group().files) {
            if (devicePaths.contains(file.devicePath)) {
                toRemove.append(tile);
                break;
            }
        }
    }
    for (ThumbnailTileWidget* tile : toRemove) {
        m_tiles.removeOne(tile);
        tile->setParent(nullptr);
        tile->deleteLater();
    }
    relayoutGrid();
    emit selectionChanged(selectedGroups());
}

void ThumbnailGridWidget::clearAll()
{
    for (ThumbnailTileWidget* tile : m_tiles) {
        tile->setParent(nullptr);
        tile->deleteLater();
    }
    m_tiles.clear();
    m_lastClicked = nullptr;
    emit selectionChanged({});
}

QList<MediaAssetGroup> ThumbnailGridWidget::selectedGroups() const
{
    QList<MediaAssetGroup> result;
    for (ThumbnailTileWidget* tile : m_tiles) {
        if (tile->isSelected()) result.append(tile->group());
    }
    return result;
}

void ThumbnailGridWidget::onTileClicked(ThumbnailTileWidget* tile, Qt::KeyboardModifiers mods)
{
    bool ctrl  = mods & Qt::ControlModifier;
    bool shift = mods & Qt::ShiftModifier;

    if (shift && m_lastClicked) {
        // Range select
        int a = m_tiles.indexOf(m_lastClicked);
        int b = m_tiles.indexOf(tile);
        if (a > b) qSwap(a, b);
        for (int i = 0; i < m_tiles.size(); ++i) {
            m_tiles[i]->setSelected(i >= a && i <= b);
        }
    } else if (ctrl) {
        // Toggle single
        tile->setSelected(!tile->isSelected());
        m_lastClicked = tile;
    } else {
        // Single select
        for (ThumbnailTileWidget* t : m_tiles) t->setSelected(false);
        tile->setSelected(true);
        m_lastClicked = tile;
    }
    emit selectionChanged(selectedGroups());
}

void ThumbnailGridWidget::relayoutGrid()
{
    // Remove all from layout without deleting
    while (m_layout->count()) {
        m_layout->takeAt(0);
    }

    int cols = columnCount();
    for (int i = 0; i < m_tiles.size(); ++i) {
        m_layout->addWidget(m_tiles[i], i / cols, i % cols);
    }

    // Ensure container is tall enough
    int rows = (m_tiles.size() + cols - 1) / cols;
    int h    = rows * (TILE_H + SPACING) + 24;
    m_container->setMinimumHeight(h);
}

int ThumbnailGridWidget::columnCount() const
{
    int availableW = viewport()->width() - 24; // margins
    int cols = qMax(1, availableW / (TILE_W + SPACING));
    return cols;
}

void ThumbnailGridWidget::resizeEvent(QResizeEvent* e)
{
    QScrollArea::resizeEvent(e);
    relayoutGrid();
}

void ThumbnailGridWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_A && (e->modifiers() & Qt::ControlModifier)) {
        for (ThumbnailTileWidget* t : m_tiles) t->setSelected(true);
        emit selectionChanged(selectedGroups());
        return;
    }
    if (e->key() == Qt::Key_Escape) {
        for (ThumbnailTileWidget* t : m_tiles) t->setSelected(false);
        emit selectionChanged(selectedGroups());
        return;
    }
    QScrollArea::keyPressEvent(e);
}

void ThumbnailGridWidget::mousePressEvent(QMouseEvent* e)
{
    // Click on empty area deselects
    if (e->button() == Qt::LeftButton) {
        QWidget* child = m_container->childAt(e->pos() - QPoint(0, verticalScrollBar()->value()));
        if (!child || child == m_container) {
            for (ThumbnailTileWidget* t : m_tiles) t->setSelected(false);
            emit selectionChanged({});
        }
    }
    QScrollArea::mousePressEvent(e);
}
