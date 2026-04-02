#include "thetaexplorer/ThumbnailTileWidget.h"
#include "thetaexplorer/ColorUtils.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QFontMetrics>
#include <QStyle>

ThumbnailTileWidget::ThumbnailTileWidget(const CameraFileInfo& info, QWidget* parent)
    : QWidget(parent)
    , m_info(info)
{
    setFixedSize(160, 180);
    setObjectName("thumbnailTile");
    setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 4);
    layout->setSpacing(3);

    // Thumbnail image area
    m_thumb = new QLabel(this);
    m_thumb->setFixedSize(148, 120);
    m_thumb->setAlignment(Qt::AlignCenter);
    m_thumb->setObjectName("tileThumb");
    m_thumb->setStyleSheet(
        "background-color: #111111; border-radius: 3px;"
    );

    // Show file type icon initially
    if (info.isVideo) {
        m_thumb->setText("▶");
        m_thumb->setStyleSheet("background:#111; border-radius:3px; color:#774dcb; font-size:28px;");
    } else if (info.isRaw) {
        m_thumb->setText("RAW");
        m_thumb->setStyleSheet("background:#111; border-radius:3px; color:#e8a838; font-size:16px; font-weight:bold;");
    } else {
        m_thumb->setText("⬜");
        m_thumb->setStyleSheet("background:#111; border-radius:3px; color:#333; font-size:28px;");
    }

    // File name
    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("tileName");
    m_nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setFixedWidth(148);

    // Elide long names
    QFontMetrics fm(m_nameLabel->font());
    m_nameLabel->setText(fm.elidedText(info.name, Qt::ElideMiddle, 148));
    m_nameLabel->setStyleSheet("color: #888888; font-size: 11px; background: transparent;");

    // Type badge
    m_typeLabel = new QLabel(this);
    m_typeLabel->setAlignment(Qt::AlignHCenter);
    m_typeLabel->setFixedWidth(148);
    m_typeLabel->setStyleSheet("color: #555555; font-size: 10px; background: transparent;");

    QString typeStr;
    if (info.isVideo) typeStr = "VIDEO";
    else if (info.isRaw) typeStr = "RAW";
    else {
        // guess extension from name
        int dot = info.name.lastIndexOf('.');
        if (dot >= 0) typeStr = info.name.mid(dot + 1).toUpper();
        else typeStr = "IMG";
    }
    m_typeLabel->setText(typeStr);

    layout->addWidget(m_thumb);
    layout->addWidget(m_nameLabel);
    layout->addWidget(m_typeLabel);
    layout->addStretch();

    updateStyleState();
}

void ThumbnailTileWidget::setThumbnail(const QPixmap& pixmap)
{
    if (pixmap.isNull()) return;
    QPixmap scaled = pixmap.scaled(
        m_thumb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
    );
    m_thumb->setPixmap(scaled);
    m_thumb->setStyleSheet("background:#111111; border-radius:3px;");
}

void ThumbnailTileWidget::setSelected(bool s)
{
    if (m_selected == s) return;
    m_selected = s;
    updateStyleState();
    update();
}

void ThumbnailTileWidget::updateStyleState()
{
    // Use dynamic property to allow QSS rules based on state
    setProperty("selected", m_selected);
    setProperty("hovered",  m_hovered);
    style()->unpolish(this);
    style()->polish(this);
}

void ThumbnailTileWidget::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor bg    = m_selected ? QColor("#2a2040") : (m_hovered ? QColor("#222222") : QColor("#1d1d1d"));
    QColor border = m_selected ? QColor("#774dcb") : (m_hovered ? QColor("#555555") : QColor("#2e2e2e"));
    int bw = m_selected ? 2 : 1;

    p.setPen(QPen(border, bw));
    p.setBrush(bg);
    p.drawRoundedRect(rect().adjusted(bw/2, bw/2, -bw/2, -bw/2), 6, 6);
}

void ThumbnailTileWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        emit clicked(this, e->modifiers());
    }
    QWidget::mousePressEvent(e);
}

void ThumbnailTileWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        emit doubleClicked(this);
    }
    QWidget::mouseDoubleClickEvent(e);
}

void ThumbnailTileWidget::enterEvent(QEnterEvent* e)
{
    m_hovered = true;
    updateStyleState();
    update();
    QWidget::enterEvent(e);
}

void ThumbnailTileWidget::leaveEvent(QEvent* e)
{
    m_hovered = false;
    updateStyleState();
    update();
    QWidget::leaveEvent(e);
}
