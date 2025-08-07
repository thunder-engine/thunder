#include <QApplication>

#include <QSurfaceFormat>
#include <QSplashScreen>
#include <QFile>

#include <stdio.h>

#include "main/mainwindow.h"

#include "screens/projectbrowser/projectbrowser.h"

#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>

#include <engine.h>

#include <global.h>
#include "qlog.h"

#include "config.h"

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

    QFile qss(":/Style/styles/dark/style.qss");
    if(qss.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    QString project;
    if(argc > 1) {
        project = QApplication::arguments().at(1);
    } else {
        ProjectBrowser browser;
        if(browser.exec() == QDialog::Accepted) {
            project = browser.projectPath();
        }
    }

    if(project.isEmpty()) {
        return 0;
    }

    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();

    File *file = new File();
    file->finit(qPrintable(QApplication::arguments().at(0)));

    Log::setLogLevel(Log::DBG);

    Engine engine(file, argv[0]);
    engine.init();
    Log::overrideHandler(new QLog());

    MainWindow window(&engine);

    splash.finish(&window);

    window.onOpenProject(project);
    window.show();

    int result  = app.exec();

    AssetManager::destroy();
    PluginManager::destroy();

    return result;
}
