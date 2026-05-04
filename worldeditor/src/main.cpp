#include <QApplication>

#include <QSurfaceFormat>
#include <QSplashScreen>
#include <QFile>

#include "main/mainwindow.h"

#include "screens/projectbrowser/projectbrowser.h"

#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/editorplatform.h>

#include <engine.h>
#include <log.h>

#include <global.h>

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
    if(qss.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    // Need to set platform adaptor first
    Engine::setOrganizationName(COMPANY_NAME);
    Engine::setApplicationName(EDITOR_NAME);
    Engine::setApplicationVersion(TString(SDK_VERSION) + " rev " + REVISION);

    Log::setLogLevel(Log::DBG);

    Engine engine;
    Engine::setPlatformAdaptor(&EditorPlatform::instance());

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

    MainWindow window(&engine);

    splash.finish(&window);

    window.onOpenProject(project);
    window.show();

    int result = app.exec();

    PluginManager::destroy();

    return result;
}
