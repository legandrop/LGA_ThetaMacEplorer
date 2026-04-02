#pragma once

#include <QString>
#include <QList>
#include "thetaexplorer/CameraFileInfo.h"

class CameraCatalogDebugExporter
{
public:
    static bool exportCatalog(const QList<CameraFileInfo>& files,
                              const QString& outputPath,
                              QString* errorMessage = nullptr);
};
