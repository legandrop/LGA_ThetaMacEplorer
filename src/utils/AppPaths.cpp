#include "thetaexplorer/AppPaths.h"
#include "thetaexplorer/Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace {
constexpr auto kAppFolder = "ThetaMacExplorer";
// Plist de las versiones que guardaban con QSettings nativo (Qt lo nombraba com.lga.<app>).
constexpr auto kLegacyPlist = "Library/Preferences/com.lga.ThetaMacExplorer.plist";
}

QString AppPaths::defaultLogFile()
{
    return QDir::homePath() + QStringLiteral("/Library/Logs/LGA/%1/debug.log").arg(kAppFolder);
}

QString AppPaths::settingsDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString AppPaths::settingsFile()
{
    return QDir(settingsDirectory()).filePath(QStringLiteral("settings.ini"));
}

void AppPaths::migrateLegacySettings()
{
    const QString iniPath = settingsFile();
    if (QFileInfo::exists(iniPath)) {
        return;
    }
    QDir().mkpath(settingsDirectory());

    const QString legacyPath = QDir::home().filePath(kLegacyPlist);
    if (!QFileInfo::exists(legacyPath)) {
        return;
    }

    QSettings legacy(legacyPath, QSettings::NativeFormat);
    QSettings ini(iniPath, QSettings::IniFormat);
    int copied = 0;
    for (const QString& key : legacy.allKeys()) {
        // El plist tambien trae claves globales del sistema (NSxxx, Apple*): solo se copian
        // las de la app.
        if (key.startsWith("window/") || key.startsWith("downloads/")) {
            ini.setValue(key, legacy.value(key));
            ++copied;
        }
    }
    ini.sync();
    LOGI("app") << "Settings migrados del plist anterior a " << iniPath
                << " (" << copied << " claves)";
}
