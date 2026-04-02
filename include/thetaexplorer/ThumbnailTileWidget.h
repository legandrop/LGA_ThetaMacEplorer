#pragma once
#include <QWidget>
#include <QLabel>
#include "thetaexplorer/CameraFileInfo.h"

class ThumbnailTileWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
public:
    explicit ThumbnailTileWidget(const CameraFileInfo& info, QWidget* parent = nullptr);

    const CameraFileInfo& fileInfo() const { return m_info; }
    bool  isSelected() const { return m_selected; }
    void  setSelected(bool s);
    void  setThumbnail(const QPixmap& pixmap);

signals:
    void clicked(ThumbnailTileWidget* self, Qt::KeyboardModifiers mods);
    void doubleClicked(ThumbnailTileWidget* self);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    void updateStyleState();

    CameraFileInfo m_info;
    QLabel*        m_thumb     = nullptr;
    QLabel*        m_nameLabel = nullptr;
    QLabel*        m_typeLabel = nullptr;
    bool           m_selected  = false;
    bool           m_hovered   = false;
};
