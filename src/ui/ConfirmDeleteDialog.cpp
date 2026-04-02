#include "thetaexplorer/ConfirmDeleteDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

ConfirmDeleteDialog::ConfirmDeleteDialog(int fileCount, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Confirm Delete");
    setModal(true);
    setFixedWidth(360);
    setStyleSheet(
        "QDialog { background: #1d1d1d; }"
        "QLabel  { color: #b2b2b2; }"
    );

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 16);
    layout->setSpacing(16);

    // Warning icon + message
    auto* row = new QHBoxLayout();
    auto* icon = new QLabel("⚠️", this);
    icon->setStyleSheet("font-size: 28px; background: transparent;");
    auto* msg = new QLabel(
        QString("Delete <b>%1 file%2</b> from the camera?<br>"
                "<span style='color:#666666;font-size:12px'>"
                "This cannot be undone. Files will be permanently removed from the device."
                "</span>")
            .arg(fileCount)
            .arg(fileCount > 1 ? "s" : ""),
        this
    );
    msg->setWordWrap(true);
    msg->setTextFormat(Qt::RichText);
    msg->setStyleSheet("color: #b2b2b2; background: transparent;");
    row->addWidget(icon, 0, Qt::AlignTop);
    row->addWidget(msg,  1);
    layout->addLayout(row);

    // Buttons
    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setStyleSheet(
        "QPushButton { background:#252525; color:#b2b2b2; border:1px solid #333; "
        "border-radius:5px; padding:6px 18px; }"
        "QPushButton:hover { background:#2e2e2e; border-color:#555; }"
    );

    auto* deleteBtn = new QPushButton("Delete", this);
    deleteBtn->setObjectName("deleteConfirmBtn");
    deleteBtn->setStyleSheet(
        "QPushButton { background:#3d1f27; color:#f0b8c0; border:1px solid #cf6679; "
        "border-radius:5px; padding:6px 18px; }"
        "QPushButton:hover { background:#4d2535; border-color:#df7080; }"
    );

    btnRow->addWidget(cancelBtn);
    btnRow->addSpacing(8);
    btnRow->addWidget(deleteBtn);
    layout->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(deleteBtn, &QPushButton::clicked, this, &QDialog::accept);

    deleteBtn->setFocus();
}

bool ConfirmDeleteDialog::confirm(int fileCount, QWidget* parent)
{
    ConfirmDeleteDialog dlg(fileCount, parent);
    return dlg.exec() == QDialog::Accepted;
}
