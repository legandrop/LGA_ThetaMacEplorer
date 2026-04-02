#include "thetaexplorer/ThumbnailTileWidget.h"
#include "thetaexplorer/ColorUtils.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QFontMetrics>
#include <QFileInfo>
#include <QDateTime>
#include <QStyle>

ThumbnailTileWidget::ThumbnailTileWidget(const MediaAssetGroup& group, QWidget* parent)
    : QWidget(parent)
    , m_group(group)
{
    setFixedSize(160, 214);
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

    m_downloadedBadge = new QLabel(m_thumb);
    m_downloadedBadge->setGeometry(110, 6, 32, 18);
    m_downloadedBadge->setAlignment(Qt::AlignCenter);
    m_downloadedBadge->setText("DL");
    m_downloadedBadge->setStyleSheet(
        "background:#2e7d32; color:#eaf7ea; border-radius:9px; "
        "font-size:9px; font-weight:bold; padding:1px 4px;"
    );
    m_downloadedBadge->setVisible(group.allFilesDownloaded);

    // Show file type icon initially
    if (group.isVideo) {
        m_thumb->setText("▶");
        m_thumb->setStyleSheet("background:#111; border-radius:3px; color:#774dcb; font-size:28px;");
    } else if (group.isRaw) {
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
    m_nameLabel->setText(fm.elidedText(group.displayTitle, Qt::ElideMiddle, 148));
    m_nameLabel->setStyleSheet("color: #888888; font-size: 11px; background: transparent;");

    // Type badge
    m_typeLabel = new QLabel(this);
    m_typeLabel->setAlignment(Qt::AlignHCenter);
    m_typeLabel->setFixedWidth(148);
    m_typeLabel->setStyleSheet("color: #555555; font-size: 10px; background: transparent;");

    m_typeLabel->setText(group.subtitle);

    m_datesLabel = new QLabel(this);
    m_datesLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_datesLabel->setFixedWidth(148);
    m_datesLabel->setWordWrap(true);
    m_datesLabel->setTextFormat(Qt::RichText);
    m_datesLabel->setStyleSheet("font-size: 8px; background: transparent;");
    m_datesLabel->setText(buildDatesHtml());

    layout->addWidget(m_thumb);
    layout->addWidget(m_nameLabel);
    layout->addWidget(m_typeLabel);
    layout->addWidget(m_datesLabel);
    layout->addStretch();

    updateStyleState();
}

QString ThumbnailTileWidget::buildDatesHtml() const
{
    QStringList chunks;
    for (const CameraFileInfo& file : m_group.files) {
        const QString color = ageColor(file.creationDate);
        const QString text = file.creationDate.isValid()
            ? file.creationDate.toString("MM-dd HH:mm")
            : QString("no-date");
        chunks.append(QString("<span style=\"color:%1;\">%2</span>").arg(color, text));
    }

    QStringList lines;
    const int perLine = m_group.isHdrSet ? 3 : 1;
    for (int i = 0; i < chunks.size(); i += perLine) {
        QStringList row;
        for (int j = i; j < qMin(i + perLine, chunks.size()); ++j) {
            row.append(chunks[j]);
        }
        lines.append(row.join("  "));
    }
    return lines.join("<br/>");
}

QString ThumbnailTileWidget::ageColor(const QDateTime& timestamp) const
{
    if (!timestamp.isValid()) {
        return "#5e5e5e";
    }

    const QDateTime now = QDateTime::currentDateTime();
    const qint64 ageSecs = timestamp.secsTo(now);
    if (ageSecs < 0) {
        return "#8ae88f";
    }

    const double ageHours = ageSecs / 3600.0;
    if (ageHours > 28.0) {
        return "#5e5e5e";
    }

    const double t = qBound(0.0, ageHours / 28.0, 1.0);
    const int startR = 192, startG = 255, startB = 192;
    const int endR = 35, endG = 102, endB = 52;
    const int r = static_cast<int>(startR + (endR - startR) * t);
    const int g = static_cast<int>(startG + (endG - startG) * t);
    const int b = static_cast<int>(startB + (endB - startB) * t);
    return QString("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0'));
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
