#include "thetaexplorer/MainWindow.h"
#include "thetaexplorer/CameraFileInfo.h"
#include "thetaexplorer/ColorUtils.h"
#include "thetaexplorer/Logger.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

int main(int argc, char* argv[])
{
    thetaexplorer::Logger::instance().initialize();

    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

    QApplication app(argc, argv);
    app.setApplicationName("ThetaMacExplorer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LGA");
    app.setOrganizationDomain("lga.com");

    LOGI("app") << "ThetaMacExplorer started at"
                << QDateTime::currentDateTime().toString()
                << "|" << thetaexplorer::Logger::instance().configurationSummary();

    // Register custom metatypes for cross-thread signal/slot use
    qRegisterMetaType<CameraFileInfo>("CameraFileInfo");
    qRegisterMetaType<QList<CameraFileInfo>>("QList<CameraFileInfo>");
    qRegisterMetaType<QPixmap>("QPixmap");

    // Load QSS from resources, fall back to inline ColorUtils stylesheet
    QFile styleFile(":/styles/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QTextStream(&styleFile).readAll());
        LOGD("app") << "QSS loaded from resources";
    } else {
        app.setStyleSheet(ColorUtils::getStyleSheet());
        LOGW("app") << "QSS loaded from ColorUtils fallback";
    }

    MainWindow window;
    window.show();

    int ret = app.exec();
    LOGI("app") << "ThetaMacExplorer exiting";
    thetaexplorer::Logger::instance().shutdown();
    return ret;
}
