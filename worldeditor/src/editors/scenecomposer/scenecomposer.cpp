#include "scenecomposer.h"
#include "ui_scenecomposer.h"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QVariant>
#include <QWidgetAction>

#include <QQmlContext>
#include <QQuickItem>
#include <QMenuBar>

#include <json.h>
#include <bson.h>

#include <cstring>

// Engine
#include <module.h>
#include <components/scene.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>
#include <components/meshrender.h>
#include <components/camera.h>

// Misc
#include "controllers/objectctrl.h"
#include "graph/sceneview.h"

#include "managers/asseteditormanager/importqueue.h"
#include "managers/feedmanager/feedmanager.h"

#include "projectmodel.h"
#include "projectmanager.h"
#include "pluginmanager.h"
#include "settingsmanager.h"
#include "undomanager.h"

#include "documentmodel.h"

// System
#include <global.h>
#include "qlog.h"
#include "config.h"

// Editors
#include "editors/propertyedit/nextobject.h"
#include "editors/componentbrowser/componentmodel.h"
#include "editors/contentbrowser/contentlist.h"
#include "editors/contentbrowser/contenttree.h"
#include "editors/assetselect/assetlist.h"

Q_DECLARE_METATYPE(Object *)
Q_DECLARE_METATYPE(Actor *)
Q_DECLARE_METATYPE(Object::ObjectList *)

namespace  {
    const QString gRecent("Recent");

    const QString gGeometry("main.geometry");
    const QString gWindows("main.windows");
    const QString gWorkspace("main.workspace");
};

SceneComposer::SceneComposer(Engine *engine, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::SceneComposer),
        m_Engine(nullptr),
        m_Properties(nullptr),
        m_CurrentWorkspace(":/Workspaces/Default.ws"),
        m_Builder(nullptr),
        m_Queue(new ImportQueue(engine)),
        m_ProjectModel(nullptr),
        m_FeedManager(nullptr),
        m_DocumentModel(nullptr),
        m_Undo(nullptr),
        m_Redo(nullptr),
        m_MainDocument(nullptr),
        m_CurrentDocument(nullptr) {

    qRegisterMetaType<Vector2>  ("Vector2");
    qRegisterMetaType<Vector3>  ("Vector3");

    qRegisterMetaType<uint8_t>  ("uint8_t");
    qRegisterMetaType<uint32_t> ("uint32_t");

    qmlRegisterType<ProjectModel>("com.frostspear.thunderengine", 1, 0, "ProjectModel");

    SettingsManager::instance()->registerProperty("Language", QLocale());

    ui->setupUi(this);

    m_Builder = new QProcess(this);
    connect(m_Builder, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(m_Builder, SIGNAL(readyReadStandardError()), this, SLOT(readError()));
    connect(m_Builder, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));

    m_Engine = engine;

    QLog *handler = static_cast<QLog *>(Log::handler());
    connect(handler, SIGNAL(postRecord(uint8_t, const QString &)), ui->consoleOutput, SLOT(onLogRecord(uint8_t, const QString &)));

    connect(m_Queue, &ImportQueue::rendered, ContentList::instance(), &ContentList::onRendered);
    connect(m_Queue, &ImportQueue::rendered, ContentTree::instance(), &ContentTree::onRendered);
    connect(m_Queue, &ImportQueue::rendered, AssetList::instance(), &AssetList::onRendered);

    ui->viewportWidget->setWindowTitle(tr("Viewport"));
    ui->propertyWidget->setWindowTitle(tr("Properties"));
    ui->projectWidget->setWindowTitle(tr("Project Settings"));
    ui->preferencesWidget->setWindowTitle(tr("Editor Preferences"));
    ui->timeline->setWindowTitle(tr("Timeline"));
    ui->classMapView->setWindowTitle(tr("Class View"));

    ObjectCtrl *ctl = new ObjectCtrl(ui->viewport);

    ctl->resetModified();

    ui->renderMode->setMenu(new QMenu);
    ctl->createMenu(ui->renderMode->menu());

    ui->viewport->setController(ctl);
    ui->viewport->setScene(m_Engine->scene());

    ui->preview->setEngine(m_Engine);
    ui->preview->setScene(m_Engine->scene());
    ui->preview->setWindowTitle("Preview");

    ui->hierarchy->setController(ctl);

    m_MainDocument = ui->viewport;

    Input::init(ui->preview);

    m_Undo = UndoManager::instance()->createUndoAction(ui->menuEdit);
    m_Undo->setShortcut(QKeySequence("Ctrl+Z"));
    ui->menuEdit->insertAction(ui->actionNew_Object, m_Undo);

    m_Redo = UndoManager::instance()->createRedoAction(ui->menuEdit);
    m_Redo->setShortcut(QKeySequence("Ctrl+Y"));
    ui->menuEdit->insertAction(ui->actionNew_Object, m_Redo);

    ui->componentButton->setProperty("blue", true);

    int index = 0;
    for(auto &it : ctl->tools()) {
        QPushButton *tool = new QPushButton();
        tool->setProperty("blue", true);
        tool->setProperty("checkred", true);
        tool->setCheckable(true);
        tool->setAutoExclusive(true);
        tool->setIcon(QIcon(it->icon()));
        tool->setObjectName(it->name());
        QString shortcut = it->shortcut();
        tool->setShortcut(QKeySequence(shortcut));
        tool->setToolTip(it->toolTip() + (!shortcut.isEmpty() ? (" (" + shortcut + ")") : ""));

        ui->viewportLayout->insertWidget(index, tool);

        connect(tool, SIGNAL(clicked()), ctl, SLOT(onChangeTool()));
        if(index == 0) {
            tool->click();
        }
        index++;
    }

    ui->orthoButton->setProperty("checkgreen", true);
    ui->commitButton->setProperty("green", true);

    connect(ui->actionBuild_All, &QAction::triggered, this, &SceneComposer::onBuildProject);

    m_ProjectModel = new ProjectModel();
    m_FeedManager = new FeedManager();

    ui->quickWidget->rootContext()->setContextProperty("projectsModel", m_ProjectModel);
    ui->quickWidget->rootContext()->setContextProperty("feedManager", m_FeedManager);
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/Startup.qml"));
    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(openProject(QString)), this, SLOT(onOpenProject(QString)));
    connect(item, SIGNAL(newProject()), this, SLOT(onNewProject()));
    connect(item, SIGNAL(importProject()), this, SLOT(onImportProject()));

    ComponentBrowser *comp  = new ComponentBrowser(this);
    QMenu *menu = new QMenu(ui->componentButton);
    QWidgetAction *action   = new QWidgetAction(menu);
    action->setDefaultWidget(comp);
    menu->addAction(action);
    ui->componentButton->setMenu(menu);
    connect(comp, SIGNAL(componentSelected(QString)), ctl, SLOT(onCreateComponent(QString)));
    connect(comp, SIGNAL(componentSelected(QString)), menu, SLOT(hide()));

    comp->setGroups(QStringList("Components"));
    ui->components->setGroups(QStringList() << "Scene" << "Components");

    connect(ui->projectWidget, &SettingsBrowser::commited, ProjectManager::instance(), &ProjectManager::saveSettings);
    connect(ui->projectWidget, &SettingsBrowser::reverted, ProjectManager::instance(), &ProjectManager::loadSettings);

    connect(ui->preferencesWidget, &SettingsBrowser::commited, SettingsManager::instance(), &SettingsManager::saveSettings);
    connect(ui->preferencesWidget, &SettingsBrowser::reverted, SettingsManager::instance(), &SettingsManager::loadSettings);

    findWorkspaces(":/Workspaces");
    findWorkspaces("workspaces");
    ui->menuWorkspace->insertSeparator(ui->actionReset_Workspace);

    ui->actionAbout->setText(tr("About %1...").arg(EDITOR_NAME));
    connect(ui->actionAbout, &QAction::triggered, &m_aboutDlg, &AboutDialog::exec);
    connect(ui->actionPlugin_Manager, &QAction::triggered, &m_pluginDlg, &PluginDialog::exec);

    connect(ui->toolWidget, &QToolWindowManager::toolWindowVisibilityChanged, this, &SceneComposer::onToolWindowVisibilityChanged);
    connect(ui->toolWidget, &QToolWindowManager::currentToolWindowChanged, this, &SceneComposer::onCurrentToolWindowChanged);

    connect(ctl, SIGNAL(mapUpdated()), ui->hierarchy, SLOT(onHierarchyUpdated()));
    connect(ctl, SIGNAL(objectsSelected(Object::ObjectList)), this, SLOT(onObjectSelected(Object::ObjectList)));
    connect(ctl, SIGNAL(objectsSelected(Object::ObjectList)), ui->hierarchy, SLOT(onObjectSelected(Object::ObjectList)));
    connect(ctl, SIGNAL(objectsSelected(Object::ObjectList)), ui->timeline, SLOT(onObjectSelected(Object::ObjectList)));
    connect(ctl, SIGNAL(loadMap(QString)), this, SLOT(onOpen(QString)));
    connect(ui->hierarchy,   SIGNAL(selected(Object::ObjectList)), ctl, SLOT(onSelectActor(Object::ObjectList)));
    connect(ui->hierarchy,   SIGNAL(removed(Object::ObjectList)), ctl, SLOT(onRemoveActor(Object::ObjectList)));
    connect(ui->hierarchy,   SIGNAL(updated()), ctl, SLOT(onUpdated()));
    connect(ui->hierarchy,   SIGNAL(parented(Object::ObjectList,Object*)), ctl, SLOT(onParentActor(Object::ObjectList,Object*)));
    connect(ui->hierarchy,   SIGNAL(focused(Object*)), ctl, SLOT(onFocusActor(Object*)));
    connect(ui->orthoButton, &QPushButton::toggled, ctl, &ObjectCtrl::onOrthographic);
    connect(ui->localButton, &QPushButton::toggled, ctl, &ObjectCtrl::onLocal);
    connect(ui->localButton, &QPushButton::toggled, this, &SceneComposer::onLocal);

    connect(PluginManager::instance(), SIGNAL(pluginReloaded()), ctl, SLOT(onUpdateSelected()));

    connect(ui->contentBrowser, &ContentBrowser::assetSelected, this, &SceneComposer::onAssetSelected);
    connect(ui->contentBrowser, &ContentBrowser::openEditor, this, &SceneComposer::onOpenEditor);

    connect(ui->timeline, SIGNAL(animated(bool)), ui->propertyView, SLOT(onAnimated(bool)));

    connect(ui->hierarchy, SIGNAL(updated()), ui->propertyView, SLOT(onUpdated()));

    ui->toolWidget->addToolWindow(ui->viewportWidget,    QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->preview,           QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->contentBrowser,    QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->hierarchy,         QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->propertyWidget,    QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->components,        QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->consoleOutput,     QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->timeline,          QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->projectWidget,     QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->preferencesWidget, QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->classMapView,      QToolWindowManager::NoArea);

    foreach(QWidget *it, ui->toolWidget->toolWindows()) {
        QAction *action = new QAction(it->windowTitle(), ui->menuWindow);
        ui->menuWindow->addAction(action);
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }

    AssetManager::ClassMap map = AssetManager::instance()->classMaps();
    if(!map.isEmpty()) {
        ui->classMapView->setModel(map.first());
    }

    connect(AssetManager::instance(), &AssetManager::buildSuccessful, ComponentModel::instance(), &ComponentModel::update);
    connect(AssetManager::instance(), &AssetManager::buildSuccessful, this, &SceneComposer::onRepickSelected);

    on_actionEditor_Mode_triggered();
    on_actionNew_triggered();
    resetGeometry();
}

SceneComposer::~SceneComposer() {
    delete m_Properties;

    delete ui;
}

void SceneComposer::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    QSize size(event->size());
    size.setHeight(size.height() - menuBar()->height());

    ui->quickWidget->setGeometry(QRect(QPoint(), size));
    ui->toolWidget->setGeometry(QRect(QPoint(), size));
}

void SceneComposer::onObjectSelected(Object::ObjectList objects) {
    if(m_Properties) {
        delete m_Properties;
        m_Properties = nullptr;
        ui->propertyView->setObject(m_Properties);
    }
    if(!objects.empty()) {
        ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());

        m_Properties = new NextObject(*objects.begin(), this);
        connect(ctl, SIGNAL(objectsUpdated()), m_Properties, SLOT(onUpdated()));
        connect(ctl, SIGNAL(objectsChanged(Object::ObjectList,QString)), ui->timeline, SLOT(onChanged(Object::ObjectList,QString)));

        connect(m_Properties, SIGNAL(deleteComponent(QString)), ctl, SLOT(onDeleteComponent(QString)));
        connect(m_Properties, SIGNAL(updated()), ui->propertyView, SLOT(onUpdated()));
        connect(m_Properties, SIGNAL(aboutToBeChanged(Object *, QString, Variant)), ctl, SLOT(onPropertyChanged(Object *, QString, Variant)), Qt::DirectConnection);
        connect(m_Properties, SIGNAL(changed(Object *, QString)), ctl, SLOT(onUpdated()));
        connect(m_Properties, SIGNAL(changed(Object *, QString)), ui->timeline, SLOT(onUpdated(Object *, QString)));

        connect(ui->timeline, SIGNAL(moved()), m_Properties, SLOT(onUpdated()));
    }

    IConverterSettings *s = dynamic_cast<IConverterSettings *>(ui->propertyView->object());
    if(s) {
        checkImportSettings(s);
    }

    ui->componentButton->setVisible(true);

    ui->commitButton->setVisible(false);
    ui->revertButton->setVisible(false);

    ui->propertyView->setObject(m_Properties);
}

void SceneComposer::onRepickSelected() {
    ObjectCtrl *ctl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    onObjectSelected(ctl->selected());
}

void SceneComposer::onAssetSelected(IConverterSettings *settings) {
    IConverterSettings *s = dynamic_cast<IConverterSettings *>(ui->propertyView->object());
    if(s && s != settings) {
        checkImportSettings(s);
        disconnect(s, &IConverterSettings::updated, this, &SceneComposer::onSettingsUpdated);
    }

    connect(settings, &IConverterSettings::updated, this, &SceneComposer::onSettingsUpdated);

    ui->componentButton->setVisible(false);

    ui->commitButton->setVisible(true);
    ui->revertButton->setVisible(true);

    ui->commitButton->setEnabled(settings->isModified());
    ui->revertButton->setEnabled(settings->isModified());

    ui->propertyView->setObject(settings);
}

void SceneComposer::onItemSelected(QObject *item) {
    ui->componentButton->setVisible(false);

    ui->commitButton->setVisible(false);
    ui->revertButton->setVisible(false);

    ui->propertyView->setObject(item);
}

void SceneComposer::onOpenEditor(const QString &path) {
    if(m_DocumentModel == nullptr) {
        return;
    }
    QWidget *editor = dynamic_cast<QWidget *>(m_DocumentModel->openFile(path));
    if(editor) {
        connect(editor, SIGNAL(templateUpdate()), ui->contentBrowser, SLOT(assetUpdated()), Qt::UniqueConnection);

        if((dynamic_cast<QMainWindow *>(editor) == nullptr)) {
            if(ui->toolWidget->areaFor(editor) == nullptr) {
                QWidget *neighbor = m_MainDocument;
                for(auto &it : findChildren<QWidget *>()) {
                    if(it->inherits(editor->metaObject()->className())) {
                        neighbor = it;
                        break;
                    }
                }

                ui->toolWidget->removeToolWindow(editor);
                editor->setParent(this);
                ui->toolWidget->addToolWindow(editor, QToolWindowManager::ReferenceAddTo, ui->toolWidget->areaFor(neighbor));
            } else {
                ui->toolWidget->activateToolWindow(editor);
            }
        } else {
            editor->show();
        }
    }
}

void SceneComposer::onSettingsUpdated() {
    ui->commitButton->setEnabled(true);
    ui->revertButton->setEnabled(true);
}

void SceneComposer::checkImportSettings(IConverterSettings *settings) {
    if(settings->isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The import settings has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return;
        }
        if(result == QMessageBox::Yes) {
            settings->saveSettings();
            AssetManager::instance()->pushToImport(settings);
            AssetManager::instance()->reimport();
        }
        if(result == QMessageBox::No) {
            settings->loadSettings();
        }
    }
}

void SceneComposer::updateTitle() {
    if(!m_Path.isEmpty()) {
        QFileInfo info(m_Path);
        setWindowTitle(info.baseName() + " - " + QString(EDITOR_NAME));
    } else {
        setWindowTitle(EDITOR_NAME);
    }
}

void SceneComposer::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);

    if(!checkSave()) {
        event->ignore();
        return;
    }

    QString str = ProjectManager::instance()->projectId();
    if(!str.isEmpty() && !m_Path.isEmpty()) {
        VariantList params;
        params.push_back(qPrintable(m_Path));
        Camera *camera  = ui->viewport->controller()->camera();
        if(camera) {
            Actor *actor = camera->actor();
            Transform *t = actor->transform();
            params.push_back(t->position());
            params.push_back(t->rotation());
            params.push_back(ui->orthoButton->isChecked());
            params.push_back(camera->focal());
            params.push_back(camera->orthoSize());
        }

        QSettings settings(COMPANY_NAME, EDITOR_NAME);
        settings.setValue(str, QString::fromStdString(Json::save(params)));
    }

    SettingsManager::instance()->saveSettings();

    saveWorkspace();
    QApplication::quit();
}

bool SceneComposer::checkSave() {
    if(static_cast<ObjectCtrl *>(ui->viewport->controller())->isModified()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The map has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result  = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return false;
        }
        if(result == QMessageBox::Yes) {
            on_actionSave_triggered();

            QString path = ProjectManager::instance()->iconPath() + "/auto.png";
            QImage result = ui->viewport->grabFramebuffer();
            QRect rect((result.width() - result.height()) / 2, 0, result.height(), result.height());
            result.copy(rect).scaled(128, 128).save(path);
        }
    }
    return true;
}

void SceneComposer::on_commitButton_clicked() {
    IConverterSettings *s = dynamic_cast<IConverterSettings *>(ui->propertyView->object());
    if(s && s->isModified()) {
        s->saveSettings();
        AssetManager::instance()->pushToImport(s);
        AssetManager::instance()->reimport();

        ui->commitButton->setEnabled(s->isModified());
        ui->revertButton->setEnabled(s->isModified());
    }
}

void SceneComposer::on_revertButton_clicked() {
    IConverterSettings *s = dynamic_cast<IConverterSettings *>(ui->propertyView->object());
    if(s && s->isModified()) {
        s->loadSettings();

        ui->commitButton->setEnabled(s->isModified());
        ui->revertButton->setEnabled(s->isModified());

        ui->propertyView->onUpdated();
    }
}

void SceneComposer::on_actionNew_triggered() {
    checkSave();

    ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    ctrl->clear();
    ctrl->setMap(Engine::objectCreate<Actor>("Chunk", m_Engine->scene()));
    ui->hierarchy->setRootObject(ctrl->map());

    UndoManager::instance()->clear();

    m_Path.clear();
    updateTitle();
}

void SceneComposer::onOpen(const QString &arg) {
    if(checkSave()) {
        if(arg.isEmpty()) {
            QString dir = ProjectManager::instance()->contentPath();
            m_Path = QFileDialog::getOpenFileName(this,
                                                  tr("Open Map"),
                                                  dir + "/maps",
                                                  tr("Maps (*.map)") );
        } else {
            m_Path = arg;
        }

        if(!m_Path.isEmpty()) {
            QFile loadFile(m_Path);
            if(!loadFile.open(QIODevice::ReadOnly)) {
                qWarning("Couldn't open file.");
                return;
            }

            QByteArray array = loadFile.readAll();
            string data;
            data.resize(array.size());
            memcpy(&data[0], array.data(), array.size());
            Variant var = Json::load(data);
            Object *map = Engine::toObject(var, ui->viewport->scene());
            if(map) {
                updateTitle();

                ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
                ctrl->clear();
                ctrl->setMap(map);

                ui->hierarchy->setRootObject(ctrl->map());

                UndoManager::instance()->clear();

                ui->toolWidget->activateToolWindow(ui->viewport);
            }
        }
    }
}

void SceneComposer::on_actionSave_triggered() {
    if(m_CurrentDocument && m_CurrentDocument != m_MainDocument) {
        m_DocumentModel->saveFile(dynamic_cast<IAssetEditor *>(m_CurrentDocument));
    } else {
        ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
        Object *map = ctrl->map();
        if(map && !ui->actionGame_Mode->isChecked()) {
            if( m_Path.length() > 0 ) {
                QDir dir = QDir(QDir::currentPath());

                string data = Json::save(Engine::toVariant(map), 0);
                if(!data.empty()) {
                    QFile file(dir.relativeFilePath(m_Path));
                    if(file.open(QIODevice::WriteOnly)) {
                        file.write(static_cast<const char *>(&data[0]), data.size());
                        file.close();
                    }
                }
                static_cast<ObjectCtrl *>(ui->viewport->controller())->resetModified();
            } else {
                on_actionSave_As_triggered();
            }
        } else {
            QApplication::beep();
        }
    }
}

void SceneComposer::on_actionSave_As_triggered() {
    if(m_CurrentDocument && m_CurrentDocument != m_MainDocument) {
        m_DocumentModel->saveFileAs(dynamic_cast<IAssetEditor *>(m_CurrentDocument));
    } else {
        QString dir = ProjectManager::instance()->contentPath();
        m_Path = QFileDialog::getSaveFileName(this,
                                              tr("Save Map"),
                                              dir + "/maps",
                                              tr("Maps (*.map)") );
        if(m_Path.length() > 0) {
            updateTitle();

            on_actionSave_triggered();
        }
    }
}

void SceneComposer::on_actionEditor_Mode_triggered() {
    ui->toolWidget->activateToolWindow(ui->viewportWidget);

    ui->actionEditor_Mode->setChecked(true);
    ui->actionGame_Mode->setChecked(false);

    Object *map = Engine::toObject(Bson::load(m_Back), nullptr);
    if(map) {
        ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
        ctrl->clear();
        ctrl->setMap(map);

        map->setParent(ui->viewport->scene()); // Set parent after detach previous one

        ui->hierarchy->setRootObject(ctrl->map());
    }

    m_Undo->setEnabled(true);
    m_Redo->setEnabled(true);

    Engine::setGameMode(false);
}

void SceneComposer::on_actionGame_Mode_triggered() {
    if(ui->actionEditor_Mode->isChecked()) {
        ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
        m_Back = Bson::save(Engine::toVariant(ctrl->map()));
        ui->toolWidget->activateToolWindow(ui->preview);
    }
    ui->actionGame_Mode->setChecked(true);
    ui->actionEditor_Mode->setChecked(false);

    m_Undo->setEnabled(false);
    m_Redo->setEnabled(false);

    Engine::setGameMode(true);
}

void SceneComposer::on_actionTake_Screenshot_triggered() {
    QImage result = ui->viewport->grabFramebuffer();
    result.save("SceneComposer-" + QDateTime::currentDateTime().toString("ddMMyy-HHmmss") + ".png");
}

void SceneComposer::onOpenProject(const QString &path) {
    connect(m_Queue, &ImportQueue::finished, this, &SceneComposer::onImportFinished, Qt::QueuedConnection);

    ui->quickWidget->stackUnder(ui->toolWidget);
    resetWorkspace();

    m_ProjectModel->addProject(path);
    ProjectManager::instance()->init(path);
    m_Engine->file()->fsearchPathAdd(qPrintable(ProjectManager::instance()->importPath()), true);

    AssetManager::instance()->rescan(!Engine::reloadBundle());
    PluginManager::instance()->rescan();

    PluginManager::instance()->initSystems();

    ui->contentBrowser->rescan();

    for(QString &it : ProjectManager::instance()->platforms()) {
        QString name = it;
        name.replace(0, 1, name.at(0).toUpper());
        QAction *action = ui->menuBuild_Project->addAction(tr("Build for %1").arg(name));
        action->setProperty(qPrintable(gPlatforms), it);
        connect(action, &QAction::triggered, this, &SceneComposer::onBuildProject);
    }

    ui->timeline->showBar();
}

void SceneComposer::onNewProject() {
    QString path = QFileDialog::getSaveFileName(this, tr("Create New Project"), ProjectManager::instance()->myProjectsPath(), "*" + gProjectExt);
    if(!path.isEmpty()) {
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            file.close();
            onOpenProject(path);
        }
    }
}

void SceneComposer::onImportProject() {
    QString path = QFileDialog::getOpenFileName(this, tr("Import Existing Project"), ProjectManager::instance()->myProjectsPath(), "*" + gProjectExt);
    if(!path.isEmpty()) {
        onOpenProject(path);
    }
}

void SceneComposer::onImportFinished() {
    m_DocumentModel = new DocumentModel;
    connect(m_DocumentModel, &DocumentModel::itemSelected, this, &SceneComposer::onItemSelected);

    CameraCtrl *ctrl = ui->viewport->controller();
    ctrl->init(ui->viewport->scene());

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant map = settings.value(ProjectManager::instance()->projectId());
    if(map.isValid()) {
        VariantList list = Json::load(map.toString().toStdString()).toList();
        if(!list.empty()) {
            auto it = list.begin();
            onOpen(it->toString().c_str());

            it++;
            Camera *camera = ctrl->camera();
            if(camera) {
                Actor *actor = camera->actor();
                Transform *t = actor->transform();
                t->setPosition(it->toVector3());
                it++;
                t->setRotation(it->toVector3());
                it++;
                ui->orthoButton->setChecked(it->toBool());
                it++;
                camera->setFocal(it->toFloat());
                it++;
                camera->setOrthoSize(it->toFloat());
                it++;
            }
        }
    }
    disconnect(m_Queue, nullptr, this, nullptr);

    ui->preferencesWidget->setModel(SettingsManager::instance());
    ui->projectWidget->setModel(ProjectManager::instance());

    ComponentModel::instance()->update();
    SettingsManager::instance()->loadSettings();

    ui->actionNew->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->menuBuild_Project->setEnabled(true);
}

void SceneComposer::on_actionUndo_triggered() {
    UndoManager::instance()->undo();
}

void SceneComposer::on_actionRedo_triggered() {
    UndoManager::instance()->redo();
}

void SceneComposer::onWorkspaceActionClicked() {
    m_CurrentWorkspace = static_cast<QAction*>(sender())->data().toString();

    on_actionReset_Workspace_triggered();
}

void SceneComposer::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->toolWidget->moveToolWindow(toolWindow, state ?
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

void SceneComposer::on_actionSave_Workspace_triggered() {
    QString path = QFileDialog::getSaveFileName(this,
                                               tr("Save Workspace"),
                                               "workspaces",
                                               tr("Workspaces (*.ws)") );
    if(path.length() > 0) {
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            QVariantMap layout;
            layout[gWindows] = ui->toolWidget->saveState();

            QByteArray data;
            QDataStream ds(&data, QIODevice::WriteOnly);
            ds << layout;

            file.write(data);
            file.close();
        }
    }
}

void SceneComposer::on_actionReset_Workspace_triggered() {
    QFile file(m_CurrentWorkspace);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QDataStream ds(&data, QIODevice::ReadOnly);
        QVariantMap layout;
        ds >> layout;
        ui->toolWidget->restoreState(layout.value(gWindows));

        foreach(auto it, ui->menuWorkspace->children()) {
            QAction *action = static_cast<QAction*>(it);
            action->blockSignals(true);
            action->setChecked((action->data().toString() == m_CurrentWorkspace));
            action->blockSignals(false);
        }
    }
}

void SceneComposer::saveWorkspace() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(gGeometry, saveGeometry());
    settings.setValue(gWindows, ui->toolWidget->saveState());
    settings.setValue(gWorkspace, m_CurrentWorkspace);
}

void SceneComposer::resetWorkspace() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant windows = settings.value(gWindows);
    m_CurrentWorkspace = settings.value(gWorkspace, m_CurrentWorkspace).toString();
    if(!windows.isValid() || !ui->toolWidget->restoreState(windows)) {
        on_actionReset_Workspace_triggered();
    } else {
        for(auto it : ui->menuWorkspace->children()) {
            QAction *action = static_cast<QAction*>(it);
            action->blockSignals(true);
            action->setChecked((action->data().toString() == m_CurrentWorkspace));
            action->blockSignals(false);
        }
    }
}

void SceneComposer::resetGeometry() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value(gGeometry).toByteArray());
}

void SceneComposer::onBuildProject() {
    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Target Directory"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly |
                                                        QFileDialog::DontResolveSymlinks);

        if(!dir.isEmpty()) {
            ProjectManager *mgr = ProjectManager::instance();

            QStringList args;
            args << "-s" << mgr->projectPath() << "-t" << dir;

            QString platform = action->property(qPrintable(gPlatforms)).toString();
            if(!platform.isEmpty()) {
                args << "-p" << platform;
            }

            qDebug() << args.join(" ");

            m_Builder->start("Builder", args);
            if(!m_Builder->waitForStarted()) {
                Log(Log::ERR) << qPrintable(m_Builder->errorString());
            }
        }
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

void SceneComposer::onFinished(int exitCode, QProcess::ExitStatus) {
    if(exitCode == 0) {
        Log(Log::INF) << "Build Finished";
    } else {
        Log(Log::ERR) << "Build Failed";
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

void SceneComposer::onLocal(bool flag) {
    ui->localButton->setIcon(flag ? QIcon(":/Style/styles/dark/icons/local.png") :
                                    QIcon(":/Style/styles/dark/icons/global.png"));
}

void SceneComposer::on_actionNew_Object_triggered() {
    ObjectCtrl *ctrl = static_cast<ObjectCtrl *>(ui->viewport->controller());
    Actor *actor = Engine::objectCreate<Actor>("NewActor", ctrl->map());
    actor->addComponent("Transform");
    actor->transform()->setPosition(0.0f);
}

void SceneComposer::findWorkspaces(const QString &dir) {
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo info(it.next());
        if(!info.baseName().isEmpty()) {
            QAction *action = new QAction(info.baseName(), ui->menuWorkspace);
            action->setCheckable(true);
            action->setData(info.filePath());
            ui->menuWorkspace->insertAction(ui->actionReset_Workspace, action);
            connect(action, SIGNAL(triggered()), this, SLOT(onWorkspaceActionClicked()));
        }
    }
}

void SceneComposer::onCurrentToolWindowChanged(QWidget *toolWindow) {
    IAssetEditor *editor = dynamic_cast<IAssetEditor *>(toolWindow);
    if(editor) {
        m_CurrentDocument = toolWindow;
    } else if(ui->viewportWidget == toolWindow) {
        m_CurrentDocument = m_MainDocument;
    }
}

void SceneComposer::on_menuFile_aboutToShow() {
    QString name;
    if(m_CurrentDocument && m_CurrentDocument != m_MainDocument) {
        name = QString(" \"%1\"").arg(m_DocumentModel->fileName(dynamic_cast<IAssetEditor *>(m_CurrentDocument)));
    }
    ui->actionSave->setText(tr("Save%1").arg(name));
    ui->actionSave_As->setText(tr("Save%1 As...").arg(name));
}

void SceneComposer::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);

        ui->viewportWidget->setWindowTitle(tr("Viewport"));
        ui->propertyWidget->setWindowTitle(tr("Properties"));
        ui->projectWidget->setWindowTitle(tr("Project Settings"));
        ui->preferencesWidget->setWindowTitle(tr("Editor Preferences"));
        ui->timeline->setWindowTitle(tr("Timeline"));
        ui->classMapView->setWindowTitle(tr("Class View"));
    }
}

void SceneComposer::on_actionReport_Issue_triggered() {
    QDesktopServices::openUrl(QUrl("https://github.com/thunder-engine/thunder/issues/new/choose", QUrl::TolerantMode));
}

void SceneComposer::on_actionAPI_Reference_triggered() {
    QDesktopServices::openUrl(QUrl("https://doc.thunderengine.org/en/latest/reference/index.html", QUrl::TolerantMode));
}
