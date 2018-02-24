#include <QApplication>

#include <QDialog>
#include <QSurfaceFormat>

#include <stdio.h>

#include "managers/undomanager/undomanager.h"

#include "editors/scenecomposer/scenecomposer.h"

#include "assetmanager.h"
#include "codemanager.h"
#include "projectmanager.h"
#include <engine.h>

#include "common.h"
#include "qlog.h"

#include <QDesktopServices>
#include <QUrl>

#include "managers/projectmanager/projectdialog.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat format;
    format.setVersion(4, 1);
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
    if(!project.isEmpty()) {
        mgr->init(project);

        IFile *file = new IFile();
        file->finit(qPrintable(QApplication::arguments().at(0)));
        file->fsearchPathAdd(qPrintable(mgr->importPath()), true);

        Engine engine(file, argc, argv);
        engine.init();

        SceneComposer w(&engine);
        QApplication::connect(AssetManager::instance(), SIGNAL(importFinished()), &w, SLOT(show()));

        CodeManager::instance()->init();
        AssetManager::instance()->init();
        UndoManager::instance()->init();

        return a.exec();
    }
    return 0;
}
