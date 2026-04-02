#include "thetaexplorer/FileMetadataPanel.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>

static QString formatSize(qint64 bytes)
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    else if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    else if (bytes < 1024 * 1024 * 1024)
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    else
        return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

FileMetadataPanel::FileMetadataPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("metadataPanel");
    setStyleSheet(
        "QWidget#metadataPanel {"
        "  background: #1a1a1a;"
        "  border-top: 1px solid #222222;"
        "  border-left: 1px solid #222222;"
        "}"
    );
    setMinimumHeight(140);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 10, 12, 10);
    outer->setSpacing(6);

    // Title
    auto* title = new QLabel("File Info", this);
    title->setStyleSheet("color: #555555; font-size: 11px; font-weight: bold; letter-spacing: 1px;");
    outer->addWidget(title);

    auto* form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(4);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setHorizontalSpacing(10);

    auto addRow = [&](const QString& key, QLabel*& val) {
        QLabel* k = makeKey(key);
        val = makeValue();
        form->addRow(k, val);
    };

    addRow("Name:",     m_nameVal);
    addRow("Type:",     m_typeVal);
    addRow("Size:",     m_sizeVal);
    addRow("Date:",     m_dateVal);
    addRow("Size (px):", m_dimVal);
    addRow("Path:",     m_pathVal);

    outer->addLayout(form);
    outer->addStretch();

    clearMetadata();
}

QLabel* FileMetadataPanel::makeKey(const QString& text)
{
    auto* l = new QLabel(text, this);
    l->setStyleSheet("color: #444444; font-size: 11px;");
    l->setFixedWidth(70);
    return l;
}

QLabel* FileMetadataPanel::makeValue()
{
    auto* l = new QLabel(this);
    l->setStyleSheet("color: #888888; font-size: 11px;");
    l->setWordWrap(false);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return l;
}

void FileMetadataPanel::setRow(QLabel* val, const QString& text)
{
    if (val) val->setText(text);
}

void FileMetadataPanel::showMetadata(const MediaAssetGroup& group)
{
    if (group.files.size() == 1) {
        const CameraFileInfo& file = group.files.first();
        setRow(m_nameVal, file.name);

        QString type;
        if (file.isVideo) type = "Video (MP4/MOV)";
        else if (file.isRaw) type = "RAW Image (DNG)";
        else {
            int dot = file.name.lastIndexOf('.');
            type = dot >= 0 ? file.name.mid(dot + 1).toUpper() + " Image" : "Image";
        }
        setRow(m_typeVal, type);
        setRow(m_sizeVal, formatSize(file.sizeBytes));
        setRow(m_dateVal, file.creationDate.isValid()
               ? file.creationDate.toString("yyyy-MM-dd  hh:mm:ss")
               : "—");
        QString dims = (file.width > 0 && file.height > 0)
                       ? QString("%1 × %2").arg(file.width).arg(file.height)
                       : "—";
        setRow(m_dimVal, dims);
        setRow(m_pathVal, file.devicePath.isEmpty() ? "—" : file.devicePath);
        return;
    }

    qint64 totalBytes = 0;
    for (const CameraFileInfo& file : group.files) {
        totalBytes += file.sizeBytes;
    }

    setRow(m_nameVal, group.displayTitle);
    setRow(m_typeVal, group.subtitle);
    setRow(m_sizeVal, formatSize(totalBytes));
    setRow(m_dateVal, group.captureTime.isValid()
           ? group.captureTime.toString("yyyy-MM-dd  hh:mm:ss")
           : "—");
    setRow(m_dimVal, QString("%1 images").arg(group.files.size()));
    setRow(m_pathVal, group.representative.devicePath.isEmpty() ? "—" : group.representative.devicePath);
}

void FileMetadataPanel::showMultipleSelection(int count)
{
    setRow(m_nameVal, QString("%1 files selected").arg(count));
    setRow(m_typeVal, "—");
    setRow(m_sizeVal, "—");
    setRow(m_dateVal, "—");
    setRow(m_dimVal,  "—");
    setRow(m_pathVal, "—");
}

void FileMetadataPanel::clearMetadata()
{
    setRow(m_nameVal, "—");
    setRow(m_typeVal, "—");
    setRow(m_sizeVal, "—");
    setRow(m_dateVal, "—");
    setRow(m_dimVal,  "—");
    setRow(m_pathVal, "—");
}
