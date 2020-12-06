#include <QApplication>

#include <QSurfaceFormat>

#include <stdio.h>

#include "undomanager.h"

#include "editors/scenecomposer/scenecomposer.h"

#include "assetmanager.h"
#include "projectmanager.h"
#include "pluginmanager.h"
#include "settingsmanager.h"

#include <engine.h>

#include <global.h>
#include "qlog.h"

#include <QDesktopServices>
#include <QUrl>

#include "editors/componentbrowser/componentmodel.h"
#include "editors/contentbrowser/contentlist.h"
#include "editors/assetselect/assetlist.h"

#include "qbsbuilder.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setVersion(4, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication::setKeyboardInputInterval(1);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(COMPANY_NAME);
    QCoreApplication::setApplicationName(EDITOR_NAME);
    QCoreApplication::setApplicationVersion(SDK_VERSION);

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

    PluginManager *plugin = PluginManager::instance();
    plugin->init(&engine);
    QApplication::connect(plugin, SIGNAL(updated()), ComponentModel::instance(), SLOT(update()));

    SettingsManager::instance()->setProperty("General/Colors/Background_Color", QColor(51, 51, 51, 0));
    SettingsManager::instance()->setProperty("General/Colors/Grid_Color", QColor(102, 102, 102, 102));
    SettingsManager::instance()->setProperty("General/Colors/Outline_Width", 1.0f);
    SettingsManager::instance()->setProperty("General/Colors/Outline_Color", QColor(255, 128, 0, 255));

    SettingsManager::instance()->setProperty("Language", QLocale());

    AssetManager *asset = AssetManager::instance();
    asset->init(&engine);
    asset->registerConverter(new QbsBuilder());

    SettingsManager::instance()->loadSettings();

    SceneComposer w(&engine);
    w.show();

    if(argc > 1) {
        w.onOpenProject(QApplication::arguments().at(1));
    }

    UndoManager::instance()->init();
    ComponentModel::instance()->init(&engine);
    ContentList::instance()->init(&engine);
    AssetList::instance()->init(&engine);

    int result  = a.exec();

    UndoManager::destroy();
    AssetManager::destroy();
    PluginManager::destroy();
    return result;
}
