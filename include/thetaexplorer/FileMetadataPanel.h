#pragma once
#include <QWidget>
#include <QLabel>
#include "thetaexplorer/CameraFileInfo.h"

class FileMetadataPanel : public QWidget
{
    Q_OBJECT
public:
    explicit FileMetadataPanel(QWidget* parent = nullptr);

    void showMetadata(const CameraFileInfo& file);
    void showMultipleSelection(int count);
    void clearMetadata();

private:
    QLabel* makeKey(const QString& text);
    QLabel* makeValue();
    void    setRow(QLabel* val, const QString& text);

    QLabel* m_nameVal  = nullptr;
    QLabel* m_typeVal  = nullptr;
    QLabel* m_sizeVal  = nullptr;
    QLabel* m_dateVal  = nullptr;
    QLabel* m_dimVal   = nullptr;
    QLabel* m_pathVal  = nullptr;
};
