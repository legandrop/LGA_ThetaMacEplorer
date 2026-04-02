#include "thetaexplorer/PreviewPanel.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QLabel>

PreviewPanel::PreviewPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("previewPanel");
    setStyleSheet("background: #131313; border-left: 1px solid #222222;");
    setMinimumWidth(200);

    m_stack = new QStackedWidget(this);

    // Page 0: image
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setObjectName("previewImage");
    m_imageLabel->setStyleSheet("background: transparent;");

    // Page 1: video placeholder
    m_videoLabel = new QLabel("▶\nVIDEO FILE", this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("color: #774dcb; font-size: 36px; background: transparent;");

    // Page 2: empty
    m_emptyLabel = new QLabel("No file selected", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #333333; font-size: 14px; background: transparent;");

    // Page 3: multi-select
    m_multiLabel = new QLabel(this);
    m_multiLabel->setAlignment(Qt::AlignCenter);
    m_multiLabel->setStyleSheet("color: #555555; font-size: 13px; background: transparent;");

    m_stack->addWidget(m_imageLabel);   // 0
    m_stack->addWidget(m_videoLabel);  // 1
    m_stack->addWidget(m_emptyLabel);  // 2
    m_stack->addWidget(m_multiLabel);  // 3
    m_stack->setCurrentIndex(2);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);
}

void PreviewPanel::showFile(const CameraFileInfo& file, const QPixmap& thumbnail)
{
    if (file.isVideo) {
        m_showingImage = false;
        m_stack->setCurrentIndex(1);
    } else if (!thumbnail.isNull()) {
        m_originalPixmap = thumbnail;
        m_showingImage = true;
        updateScaledImage();
        m_stack->setCurrentIndex(0);
    } else {
        // No thumbnail yet - show empty
        m_showingImage = false;
        m_stack->setCurrentIndex(2);
    }
}

void PreviewPanel::showMultipleSelection(int count)
{
    m_showingImage = false;
    m_multiLabel->setText(QString("%1 files selected").arg(count));
    m_stack->setCurrentIndex(3);
}

void PreviewPanel::clearPreview()
{
    m_showingImage = false;
    m_originalPixmap = QPixmap();
    m_imageLabel->clear();
    m_stack->setCurrentIndex(2);
}

void PreviewPanel::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    if (m_showingImage) updateScaledImage();
}

void PreviewPanel::updateScaledImage()
{
    if (m_originalPixmap.isNull()) return;
    QSize available = m_imageLabel->size().isEmpty() ? size() : m_imageLabel->size();
    QPixmap scaled = m_originalPixmap.scaled(
        available, Qt::KeepAspectRatio, Qt::SmoothTransformation
    );
    m_imageLabel->setPixmap(scaled);
}
