#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QSettings>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QVariant>
#include <QWidgetAction>

#include <ajson.h>
#include <abson.h>

// Engine
#include <module.h>
#include <timer.h>
#include <components/chunk.h>
#include <components/actor.h>
#include <components/sprite.h>
#include <components/staticmesh.h>

#include <analytics/profiler.h>

// Misc
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "managers/undomanager/undomanager.h"
#include "managers/pluginmanager/plugindialog.h"
#include "managers/configmanager/configdialog.h"

#include "assetmanager.h"
#include "projectmanager.h"

// System
#include "common.h"
#include "qlog.h"

// Editors
#include "editors/propertyedit/nextobject.h"
#include "editors/contentbrowser/contentlist.h"
#include "editors/componentbrowser/componentmodel.h"
#include "editors/textureedit/textureedit.h"
#include "editors/materialedit/materialedit.h"
#include "editors/meshedit/meshedit.h"

#include "managers/asseteditormanager/importqueue.h"
#include "managers/asseteditormanager/iconrender.h"

#define FPS         "FPS"
#define VERTICES    "Vertices"
#define POLYGONS    "Polygons"
#define DRAWCALLS   "Draw Calls"

Q_DECLARE_METATYPE(AObject *)
Q_DECLARE_METATYPE(Actor *)

SceneComposer::SceneComposer(Engine *engine, QWidget *parent) :
        QMainWindow(parent),
        m_pImportQueue(new ImportQueue()),
        ui(new Ui::SceneComposer) {

    qRegisterMetaType<Vector2>  ("Vector2");
    qRegisterMetaType<Vector3>  ("Vector3");

    qRegisterMetaType<uint8_t>  ("uint8_t");
    qRegisterMetaType<uint32_t> ("uint32_t");

    resetModified();

    ui->setupUi(this);

    m_pEngine   = engine;

    QLog *log   = new QLog();
    connect(log, SIGNAL(postRecord(uint8_t, const QString &)), ui->consoleOutput, SLOT(onLogRecord(uint8_t, const QString &)));
    Log::overrideHandler(log);
    Log::setLogLevel(Log::DBG);

    cmToolbars      = new QMenu(this);

    m_pPluginDlg    = new PluginDialog(m_pEngine, this);
    connect(m_pPluginDlg, SIGNAL(updated()), ComponentModel::instance(), SLOT(update()));

    glWidget        = new SceneView(m_pEngine, this);

    ObjectCtrl *ctl = new ObjectCtrl(m_pEngine, glWidget);
    connect(glWidget, SIGNAL(drop(QDropEvent*)), ctl, SLOT(onDrop()));
    connect(glWidget, SIGNAL(dragEnter(QDragEnterEvent *)), ctl, SLOT(onDragEnter(QDragEnterEvent *)));
    connect(glWidget, SIGNAL(dragLeave(QDragLeaveEvent *)), ctl, SLOT(onDragLeave(QDragLeaveEvent *)));

    glWidget->setController(ctl);
    glWidget->setObjectName("Viewport");
    glWidget->setWindowTitle("Viewport");

    ui->propertyWidget->setWindowTitle("Properties");
    ui->projectWidget->setWindowTitle("Project Settings");

    ui->commitButton->setProperty("green", true);
    ui->componentButton->setProperty("blue", true);

    ComponentBrowser *comp  = new ComponentBrowser(this);
    QMenu *menu = new QMenu(ui->componentButton);
    QWidgetAction *action   = new QWidgetAction(menu);
    action->setDefaultWidget(comp);
    menu->addAction(action);
    ui->componentButton->setMenu(menu);
    connect(comp, SIGNAL(componentSelected(QString)), ctl, SLOT(onComponentSelected(QString)));
    connect(comp, SIGNAL(componentSelected(QString)), menu, SLOT(hide()));

    comp->setGroups(QStringList("Components"));
    ui->components->setGroups(QStringList() << "Scene" << "Components");

    connect(ui->commitButton, SIGNAL(clicked(bool)), ProjectManager::instance(), SLOT(saveSettings()));

    connect(glWidget, SIGNAL(inited()), this, SLOT(onGLInit()));
    startTimer(16);

    ui->centralwidget->addToolWindow(glWidget, QToolWindowManager::EmptySpaceArea);
    ui->centralwidget->addToolWindow(ui->contentBrowser, QToolWindowManager::ReferenceBottomOf, ui->centralwidget->areaFor(glWidget));
    ui->centralwidget->addToolWindow(ui->hierarchy, QToolWindowManager::ReferenceRightOf, ui->centralwidget->areaFor(glWidget));
    ui->centralwidget->addToolWindow(ui->propertyWidget, QToolWindowManager::ReferenceBottomOf, ui->centralwidget->areaFor(ui->hierarchy));
    ui->centralwidget->addToolWindow(ui->components, QToolWindowManager::ReferenceLeftOf, ui->centralwidget->areaFor(glWidget));
    ui->centralwidget->addToolWindow(ui->consoleOutput, QToolWindowManager::ReferenceRightOf, ui->centralwidget->areaFor(ui->contentBrowser));
    ui->centralwidget->addToolWindow(ui->projectWidget, QToolWindowManager::NoArea);

    foreach(QWidget *it, ui->centralwidget->toolWindows()) {
        QAction *action = new QAction(it->windowTitle(), ui->menuWindow);
        ui->menuWindow->insertAction(ui->actionSave_Layout, action);
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }
    ui->menuWindow->insertSeparator(ui->actionSave_Layout);

    connect(ui->centralwidget, SIGNAL(toolWindowVisibilityChanged(QWidget *, bool)), this, SLOT(onToolWindowVisibilityChanged(QWidget *, bool)));

    connect(ctl, SIGNAL(mapUpdated()), ui->hierarchy, SLOT(onHierarchyUpdated()));
    connect(ctl, SIGNAL(objectsSelected(AObject::ObjectList)), this, SLOT(onObjectSelected(AObject::ObjectList)));
    connect(ctl, SIGNAL(objectsSelected(AObject::ObjectList)), ui->hierarchy, SLOT(onSelected(AObject::ObjectList)));
    connect(ctl, SIGNAL(mapUpdated()), this, SLOT(onModified()));
    connect(ctl, SIGNAL(objectsUpdated()), this, SLOT(onModified()));
    connect(ui->hierarchy, SIGNAL(selected(AObject::ObjectList)), ctl, SLOT(onSelectActor(AObject::ObjectList)));
    connect(ui->hierarchy, SIGNAL(removed(AObject::ObjectList)), ctl, SLOT(onRemoveActor(AObject::ObjectList)));
    connect(ui->hierarchy, SIGNAL(parented(AObject::ObjectList, AObject::ObjectList)), ctl, SLOT(onParentActor(AObject::ObjectList,AObject::ObjectList)));
    connect(ui->hierarchy, SIGNAL(focused(AObject*)), ctl, SLOT(onFocusActor(AObject*)));

    connect(UndoManager::instance(), SIGNAL(updated()), this, SLOT(onUndoRedoUpdated()));

    connect(ui->hierarchy, SIGNAL(updated()), ui->propertyView, SLOT(onUpdated()));
    connect(ui->hierarchy, SIGNAL(updated()), this, SLOT(onModified()));

    connect(m_pImportQueue, SIGNAL(rendered(QString)), ContentList::instance(), SLOT(onRendered(QString)));

    m_pProperties   = nullptr;
    m_pMap          = nullptr;

    on_actionEditor_Mode_triggered();
    on_actionResore_Layout_triggered();
}

SceneComposer::~SceneComposer() {
    AssetManager::destroy();

    delete m_pProperties;

    delete cmToolbars;

    delete ui;
}

void SceneComposer::timerEvent(QTimerEvent *event) {
    Timer::update();
    glWidget->update();
}

void SceneComposer::onObjectSelected(AObject::ObjectList objects) {
    if(m_pProperties) {
        delete m_pProperties;
        m_pProperties   = 0;
    }
    if(!objects.empty()) {
        glWidget->makeCurrent();
        /// \todo Don't reload mesh and materials for each repick
        m_pProperties   = new NextObject(*objects.begin(), static_cast<ObjectCtrl *>(glWidget->controller()), this);
        connect(glWidget->controller(), SIGNAL(objectsUpdated()), m_pProperties, SLOT(onUpdated()));
        connect(m_pPluginDlg, SIGNAL(pluginReloaded()), m_pProperties, SLOT(onUpdated()));
        connect(m_pProperties, SIGNAL(updated()), ui->propertyView, SLOT(onUpdated()));
        connect(m_pProperties, SIGNAL(updated()), this, SLOT(onModified()));
    }
    ui->propertyView->setObject(m_pProperties);
}

void SceneComposer::onGLInit() {
    m_pImportQueue->init(new IconRender(m_pEngine, glWidget->context()));

    AssetManager *asset = AssetManager::instance();
    asset->addEditor(IConverter::ContentTexture, new TextureEdit(m_pEngine));
    asset->addEditor(IConverter::ContentMaterial, new MaterialEdit(m_pEngine));
    asset->addEditor(IConverter::ContentMesh, new MeshEdit(m_pEngine));
    //asset->addEditor(IConverter::ContentEffect, new ParticleEdit(m_pEngine));

    ComponentModel::instance()->init(m_pEngine);
    ContentList::instance()->init(m_pEngine);

    ui->projectSettings->setObject(ProjectManager::instance());

    on_action_New_triggered();
}

void SceneComposer::updateTitle() {
    if(!mPath.isEmpty()) {
        setWindowTitle(mPath + " - " + QString(EDITOR_NAME));
    } else {
        setWindowTitle(EDITOR_NAME);
    }
}

void SceneComposer::closeEvent(QCloseEvent *event) {
    if(!checkSave()) {
        event->ignore();
    } else {
        QMainWindow::closeEvent(event);
    }
}

bool SceneComposer::checkSave() {
    if(isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("The map has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return false;
        }
        if(result == QMessageBox::Yes) {
            on_actionSave_triggered();
        }
    }
    return true;
}

void SceneComposer::on_action_New_triggered() {
    checkSave();

    delete m_pMap;

    m_pMap  = Engine::objectCreate<Chunk>("Chunk", glWidget->scene());
    ui->hierarchy->setObject(m_pMap);

    ObjectCtrl * ctrl   = static_cast<ObjectCtrl *>(glWidget->controller());

    ctrl->clear();
    ctrl->setMap(m_pMap);

    UndoManager::instance()->clear();

    mPath.clear();
}

void SceneComposer::on_action_Open_triggered() {
    checkSave();

    QString dir = ProjectManager::instance()->contentPath();
    mPath       = QFileDialog::getOpenFileName(this,
                                               tr("Open Map"),
                                               dir + "/maps",
                                               tr("Maps (*.map)") );
    if( !mPath.isEmpty() ) {
        QFile loadFile(mPath);
        if (!loadFile.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open file.");
            return;
        }

        QByteArray array    = loadFile.readAll();
        string data;
        data.resize(array.size());
        memcpy(&data[0], array.data(), array.size());
        AVariant var    = AJson::load(data);
        AObject *map    = Engine::toObject(var);
        if(map) {
            updateTitle();

            map->setParent(glWidget->scene());
            delete m_pMap;
            m_pMap  = map;
            ui->hierarchy->setObject(m_pMap);
            ObjectCtrl * ctrl   = static_cast<ObjectCtrl *>(glWidget->controller());

            ctrl->clear();
            ctrl->setMap(m_pMap);

            UndoManager::instance()->clear();
        }
    }
}

void SceneComposer::on_actionSave_triggered() {
    if(m_pMap) {
        if( mPath.length() > 0 ) {
            QDir dir    = QDir(QDir::currentPath());

            QFile file(dir.relativeFilePath(mPath));
            if(file.open(QIODevice::WriteOnly)) {
                string data = AJson::save(Engine::toVariant(m_pMap), 0);
                file.write((const char *)&data[0], data.size());
                file.close();
            }
            resetModified();
        } else {
            on_actionSave_As_triggered();
        }
    }
}

void SceneComposer::on_actionSave_As_triggered() {
    QString dir = ProjectManager::instance()->contentPath();
    mPath       = QFileDialog::getSaveFileName(this,
                                               tr("Save Map"),
                                               dir + "/maps",
                                               tr("Maps (*.map)") );
    if( mPath.length() > 0 ) {
        updateTitle();

        on_actionSave_triggered();
    }
}

void SceneComposer::on_actionPlugin_Manager_triggered() {
    m_pPluginDlg->exec();
}

void SceneComposer::on_actionEditor_Mode_triggered() {
    ui->actionEditor_Mode->setChecked(true);
    ui->actionGame_Mode->setChecked(false);

    //m_pEngine->setMode(Engine::EDIT);
}

void SceneComposer::on_actionGame_Mode_triggered() {
    ui->actionGame_Mode->setChecked(true);
    ui->actionEditor_Mode->setChecked(false);

    //m_pEngine->setMode(Engine::GAME);
}

void SceneComposer::on_actionTake_Screenshot_triggered() {
    QImage result   = glWidget->grabFramebuffer();
    result.save("SceneComposer-" + QDateTime::currentDateTime().toString("ddMMyy-HHmmss") + ".png");
}

void SceneComposer::onUndoRedoUpdated() {
    UndoManager *mgr    = UndoManager::instance();
    QString Undo    = mgr->undoTop();
    QString Redo    = mgr->redoTop();

    ui->actionUndo->setDisabled( Undo.isEmpty() );
    ui->actionRedo->setDisabled( Redo.isEmpty() );

    ui->actionUndo->setText(tr("Undo ") + Undo);
    ui->actionRedo->setText(tr("Redo ") + Redo);
}

void SceneComposer::on_actionUndo_triggered() {
    UndoManager::instance()->undo();
}

void SceneComposer::on_actionRedo_triggered() {
    UndoManager::instance()->redo();
}

void SceneComposer::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->centralwidget->moveToolWindow(toolWindow, state ?
                                              QToolWindowManager::NewFloatingArea :
                                              QToolWindowManager::NoArea);
}

void SceneComposer::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}

void SceneComposer::on_actionSave_Layout_triggered() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue("main.geometry", saveGeometry());
    settings.setValue("main.windows", ui->centralwidget->saveState());
}

void SceneComposer::on_actionResore_Layout_triggered() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry (settings.value("main.geometry").toByteArray());
    ui->centralwidget->restoreState(settings.value("main.windows"));
}

void SceneComposer::onModified() {
    mModified   = true;
}

void SceneComposer::on_actionBuild_Project_triggered() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Target Directory"),
                                                 "",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()) {
        ProjectManager *mgr = ProjectManager::instance();

        QProcess *builder   = new QProcess(this);

        QStringList args;
        args << "-s" << mgr->projectPath() << "-t" << dir;

        builder->start("Builder", args);
        if(!builder->waitForStarted()) {
            qDebug() << builder->errorString();
        }

        connect( builder, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
        connect( builder, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );
    }
}

void SceneComposer::readOutput() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardOutput());
    }
}

void SceneComposer::readError() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardError());
    }
}

void SceneComposer::parseLogs(const QString &log) {
    QStringList list = log.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" error ")) {
            Log(Log::ERR) << qPrintable(it);
        } else if(it.contains(" warning ")) {
            Log(Log::WRN) << qPrintable(it);
        } else {
            Log(Log::INF) << qPrintable(it);
        }
    }
}

void SceneComposer::on_actionOptions_triggered() {
    ConfigDialog dlg;
    dlg.exec();
}
