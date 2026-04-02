#include "thetaexplorer/MainWindow.h"
#include "thetaexplorer/CameraFileInfo.h"
#include "thetaexplorer/ColorUtils.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <stdio.h>

// ---- File logger (same pattern as LGA_PipeSync_2) ----
static QFile   g_logFile;
static QMutex  g_logMutex;

static void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg)
{
    QMutexLocker lock(&g_logMutex);

    QString level;
    switch (type) {
        case QtDebugMsg:    level = "DBG"; break;
        case QtInfoMsg:     level = "INF"; break;
        case QtWarningMsg:  level = "WRN"; break;
        case QtCriticalMsg: level = "CRT"; break;
        case QtFatalMsg:    level = "FAT"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString line = QString("[%1] [%2] %3\n").arg(timestamp, level, msg);

    // Write to file
    if (g_logFile.isOpen()) {
        g_logFile.write(line.toUtf8());
        g_logFile.flush();
    }

    // Also print to stderr so Xcode / terminal still see output
    fprintf(stderr, "%s", line.toUtf8().constData());

    if (type == QtFatalMsg) abort();
}

int main(int argc, char* argv[])
{
    // ---- Setup log file BEFORE QApplication ----
    g_logFile.setFileName("/tmp/ThetaMacExplorer.log");
    if (g_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qInstallMessageHandler(messageHandler);
    }

    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

    QApplication app(argc, argv);
    app.setApplicationName("ThetaMacExplorer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LGA");
    app.setOrganizationDomain("lga.com");

    qDebug() << "=== ThetaMacExplorer started ===" << QDateTime::currentDateTime().toString();

    // Register custom metatypes for cross-thread signal/slot use
    qRegisterMetaType<CameraFileInfo>("CameraFileInfo");
    qRegisterMetaType<QList<CameraFileInfo>>("QList<CameraFileInfo>");
    qRegisterMetaType<QPixmap>("QPixmap");

    // Load QSS from resources, fall back to inline ColorUtils stylesheet
    QFile styleFile(":/styles/dark_theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QTextStream(&styleFile).readAll());
        qDebug() << "[main] QSS loaded from resources";
    } else {
        app.setStyleSheet(ColorUtils::getStyleSheet());
        qDebug() << "[main] QSS loaded from ColorUtils fallback";
    }

    MainWindow window;
    window.show();

    int ret = app.exec();
    qDebug() << "=== ThetaMacExplorer exiting ===";
    return ret;
}
