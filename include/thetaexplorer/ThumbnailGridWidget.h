#pragma once
#include <QScrollArea>
#include <QGridLayout>
#include <QList>
#include "thetaexplorer/CameraFileInfo.h"

class ThumbnailTileWidget;

class ThumbnailGridWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit ThumbnailGridWidget(QWidget* parent = nullptr);

    void setFiles(const QList<CameraFileInfo>& files);
    void setThumbnail(const QString& devicePath, const QPixmap& thumbnail);
    void removeFiles(const QStringList& devicePaths);
    void clearAll();

    QList<CameraFileInfo> selectedFiles() const;

signals:
    void selectionChanged(const QList<CameraFileInfo>& selected);
    void thumbnailsNeeded(const QList<CameraFileInfo>& files);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onTileClicked(ThumbnailTileWidget* tile, Qt::KeyboardModifiers mods);

private:
    void relayoutGrid();
    int  columnCount() const;

    QWidget*                     m_container  = nullptr;
    QGridLayout*                 m_layout     = nullptr;
    QList<ThumbnailTileWidget*>  m_tiles;
    ThumbnailTileWidget*         m_lastClicked = nullptr;

    static constexpr int TILE_W  = 160;
    static constexpr int TILE_H  = 180;
    static constexpr int SPACING = 8;
};
