#pragma once
#include <QDialog>

class ConfirmDeleteDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfirmDeleteDialog(int fileCount, QWidget* parent = nullptr);

    // Returns true if the user confirmed deletion
    static bool confirm(int fileCount, QWidget* parent = nullptr);
};
