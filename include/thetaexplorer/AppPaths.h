#pragma once
#include <QString>

// Rutas de datos de la app, con las ubicaciones de las apps LGA:
//   logs      ~/Library/Logs/LGA/ThetaMacExplorer/debug.log
//   settings  ~/Library/Application Support/LGA/ThetaMacExplorer/settings.ini
namespace AppPaths {
    // No depende de QApplication: el Logger la usa antes de que exista.
    QString defaultLogFile();

    // Requiere organizationName/applicationName ya seteados.
    QString settingsDirectory();
    QString settingsFile();

    // Una sola vez: si todavia no hay settings.ini, copia lo que haya en el plist que usaban
    // las versiones anteriores (QSettings nativo). No borra el plist.
    void migrateLegacySettings();
}
