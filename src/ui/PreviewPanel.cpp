#include "thetaexplorer/PreviewPanel.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QLabel>
#include "thetaexplorer/UiIcons.h"

namespace {
constexpr auto kVideoColor = "#8ab4ff";   // mismo azul que la etiqueta VIDEO del tile
}

PreviewPanel::PreviewPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("previewPanel");
    setAttribute(Qt::WA_StyledBackground);
    setMinimumWidth(200);

    m_stack = new QStackedWidget(this);

    // Page 0: image
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setObjectName("previewImage");

    // Page 1: video placeholder
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setPixmap(UiIcons::pixmap("play", 48, QColor(kVideoColor), devicePixelRatioF()));

    // Page 2: empty
    m_emptyLabel = new QLabel("No item selected", this);
    m_emptyLabel->setObjectName("previewPlaceholder");
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    // Page 3: multi-select
    m_multiLabel = new QLabel(this);
    m_multiLabel->setObjectName("previewPlaceholder");
    m_multiLabel->setAlignment(Qt::AlignCenter);

    m_stack->addWidget(m_imageLabel);   // 0
    m_stack->addWidget(m_videoLabel);  // 1
    m_stack->addWidget(m_emptyLabel);  // 2
    m_stack->addWidget(m_multiLabel);  // 3
    m_stack->setCurrentIndex(2);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->addWidget(m_stack);
}

void PreviewPanel::showGroup(const MediaAssetGroup& group, const QPixmap& thumbnail)
{
    if (group.isVideo) {
        m_showingImage = false;
        m_stack->setCurrentIndex(1);
    } else if (!thumbnail.isNull()) {
        m_originalPixmap = thumbnail;
        m_showingImage = true;
        m_stack->setCurrentIndex(0);
        updateScaledImage();
    } else {
        // No thumbnail yet - show empty
        m_showingImage = false;
        m_stack->setCurrentIndex(2);
    }
}

void PreviewPanel::showMultipleSelection(int count)
{
    m_showingImage = false;
    m_multiLabel->setText(QString("%1 items selected").arg(count));
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
    // El stack siempre tiene geometria; el label de una pagina que nunca se mostro todavia
    // tiene el tamano por defecto (100x30) y la primera preview salia diminuta.
    QSize available = m_stack->size().isEmpty() ? size() : m_stack->size();
    QPixmap scaled = m_originalPixmap.scaled(
        available, Qt::KeepAspectRatio, Qt::SmoothTransformation
    );
    m_imageLabel->setPixmap(scaled);
}
