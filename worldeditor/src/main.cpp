#include <QApplication>

#include <QDialog>
#include <QSurfaceFormat>
#include <QOpenGLContext>

#include <stdio.h>

#include "managers/undomanager/undomanager.h"

#include "editors/scenecomposer/scenecomposer.h"

#include "assetmanager.h"
#include "projectmanager.h"
#include <engine.h>

#include <global.h>
#include "qlog.h"

#include <QDesktopServices>
#include <QUrl>

#include "managers/projectmanager/projectdialog.h"

#include "pluginmodel.h"

#include "editors/textureedit/textureedit.h"
#include "editors/materialedit/materialedit.h"
#include "editors/meshedit/meshedit.h"
#include "editors/particleedit/particleedit.h"
#include "editors/animationedit/animationedit.h"

#include "editors/componentbrowser/componentmodel.h"
#include "editors/contentbrowser/contentlist.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setVersion(4, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setRenderableType(QSurfaceFormat::OpenGLES);
    QSurfaceFormat::setDefaultFormat(format);

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

    IFile *file = new IFile();
    file->finit(qPrintable(QApplication::arguments().at(0)));

    Log::setLogLevel(Log::DBG);

    Engine engine(file, argc, argv);
    engine.init();
    Log::overrideHandler(new QLog());

    PluginModel *plugin = PluginModel::instance();
    plugin->init(&engine);
    QApplication::connect(plugin, SIGNAL(updated()), ComponentModel::instance(), SLOT(update()));

    AssetManager *asset = AssetManager::instance();
    asset->addEditor(IConverter::ContentTexture, new TextureEdit(&engine));
    asset->addEditor(IConverter::ContentMaterial, new MaterialEdit(&engine));
    asset->addEditor(IConverter::ContentMesh, new MeshEdit(&engine));
    asset->addEditor(IConverter::ContentEffect, new ParticleEdit(&engine));
    asset->addEditor(IConverter::ContentAnimationStateMachine, new AnimationEdit(&engine));

    SceneComposer w(&engine);
    w.show();

    if(argc > 1) {
        w.onOpenProject(QApplication::arguments().at(1));
    }

    UndoManager::instance()->init();
    ComponentModel::instance()->init(&engine);
    ContentList::instance()->init(&engine);

    int result  = a.exec();

    asset->destroy();
    plugin->destroy();

    UndoManager::destroy();
    AssetManager::destroy();
    return result;
}
