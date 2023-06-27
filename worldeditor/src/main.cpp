#include <QApplication>

#include <QSurfaceFormat>
#include <QSplashScreen>

#include <stdio.h>

#include "main/mainwindow.h"

#include "assetmanager.h"

#include <editor/undomanager.h>
#include <editor/pluginmanager.h>
#include <editor/settingsmanager.h>

#include <engine.h>

#include <global.h>
#include "qlog.h"

#include "config.h"

#include "editors/componentbrowser/componentmodel.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setVersion(4, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication::setKeyboardInputInterval(1);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setApplicationName(EDITOR_NAME);
    QCoreApplication::setApplicationVersion(SDK_VERSION);

    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();

    QFile qss(":/Style/styles/dark/style.qss");
    if(qss.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    File *file = new File();
    file->finit(qPrintable(QApplication::arguments().at(0)));

    Log::setLogLevel(Log::DBG);

    Engine engine(file, argv[0]);
    engine.init();
    Log::overrideHandler(new QLog());

    PluginManager::instance()->init(&engine);
    AssetManager::instance()->init();

    SettingsManager::instance()->loadSettings();

    MainWindow window(&engine);

    window.show();
    splash.finish(&window);

    if(argc > 1) {
        window.onOpenProject(QApplication::arguments().at(1));
    }

    ComponentModel::instance()->init(&engine);

    int result  = app.exec();

    UndoManager::destroy();
    AssetManager::destroy();
    PluginManager::destroy();

    return result;
}
