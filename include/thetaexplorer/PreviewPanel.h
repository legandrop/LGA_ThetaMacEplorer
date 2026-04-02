#pragma once
#include <QWidget>
#include <QLabel>
#include <QStackedWidget>
#include "thetaexplorer/CameraFileInfo.h"

class PreviewPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewPanel(QWidget* parent = nullptr);

    void showFile(const CameraFileInfo& file, const QPixmap& thumbnail);
    void showMultipleSelection(int count);
    void clearPreview();

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    void updateScaledImage();

    QStackedWidget* m_stack       = nullptr;
    QLabel*         m_imageLabel  = nullptr;   // page 0
    QLabel*         m_videoLabel  = nullptr;   // page 1
    QLabel*         m_emptyLabel  = nullptr;   // page 2
    QLabel*         m_multiLabel  = nullptr;   // page 3

    QPixmap         m_originalPixmap;
    bool            m_showingImage = false;
};
