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

#include "editors/componentbrowser/componentmodel.h"
#include "editors/contentbrowser/contentlist.h"

#include "managers/asseteditormanager/importqueue.h"

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

    ProjectManager *mgr = ProjectManager::instance();

    QString project;
    if(argc > 1) {
        project = QApplication::arguments().at(1);
    } else {
        project = ProjectDialog::projectPath();
    }
    int result = 0;
    if(!project.isEmpty()) {
        mgr->init(project);

        IFile *file = new IFile();
        file->finit(qPrintable(QApplication::arguments().at(0)));
        file->fsearchPathAdd(qPrintable(mgr->importPath()), true);

        Log::setLogLevel(Log::DBG);

        Engine engine(file, argc, argv);
        engine.init();
        engine.reloadBundle();
        Log::overrideHandler(new QLog());

        PluginModel::instance()->init(&engine);
        PluginModel::instance()->rescan();

        QApplication::connect(PluginModel::instance(), SIGNAL(updated()), ComponentModel::instance(), SLOT(update()));

        AssetManager *asset = AssetManager::instance();
        asset->addEditor(IConverter::ContentTexture, new TextureEdit(&engine));
        asset->addEditor(IConverter::ContentMaterial, new MaterialEdit(&engine));
        asset->addEditor(IConverter::ContentMesh, new MeshEdit(&engine));
        asset->addEditor(IConverter::ContentEffect, new ParticleEdit(&engine));

        ImportQueue queue(&engine);
        QApplication::connect(&queue, SIGNAL(rendered(QString)), ContentList::instance(), SLOT(onRendered(QString)));

        SceneComposer w(&engine);
        QApplication::connect(&queue, &ImportQueue::finished, &w, &SceneComposer::show);

        asset->init(&engine);
        UndoManager::instance()->init();

        ComponentModel::instance()->init(&engine);
        ContentList::instance()->init(&engine);

        result  = a.exec();

        asset->destroy();
    }
    UndoManager::destroy();
    AssetManager::destroy();
    return result;
}
