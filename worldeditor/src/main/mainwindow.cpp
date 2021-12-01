#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QFileDialog>
#include <QVariant>
#include <QWidgetAction>

#include <QQmlContext>
#include <QQuickItem>

#include <json.h>
#include <timer.h>

#include <cstring>

#include <editor/asseteditor.h>

// Misc
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
#include "config.h"

// Editors
#include "editors/propertyedit/nextobject.h"
#include "editors/componentbrowser/componentmodel.h"
#include "editors/contentbrowser/contentlist.h"
#include "editors/contentbrowser/contenttree.h"
#include "editors/assetselect/assetlist.h"

Q_DECLARE_METATYPE(Object *)
Q_DECLARE_METATYPE(Object::ObjectList *)

namespace  {
    const char *gGeometry("main.geometry");
    const char *gWindows("main.windows");
    const char *gWorkspace("main.workspace");
};

MainWindow::MainWindow(Engine *engine, QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        m_Engine(engine),
        m_CurrentWorkspace(":/Workspaces/Default.ws"),
        m_Queue(new ImportQueue(engine)),
        m_ProjectModel(new ProjectModel),
        m_FeedManager(new FeedManager),
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

    connect(m_Queue, &ImportQueue::rendered, ContentList::instance(), &ContentList::onRendered);
    connect(m_Queue, &ImportQueue::rendered, ContentTree::instance(), &ContentTree::onRendered);
    connect(m_Queue, &ImportQueue::rendered, AssetList::instance(), &AssetList::onRendered);

    ui->viewportWidget->setWindowTitle(tr("Viewport"));
    ui->propertyWidget->setWindowTitle(tr("Properties"));
    ui->projectWidget->setWindowTitle(tr("Project Settings"));
    ui->preferencesWidget->setWindowTitle(tr("Editor Preferences"));
    ui->timeline->setWindowTitle(tr("Timeline"));
    ui->classMapView->setWindowTitle(tr("Class View"));
    ui->preview->setWindowTitle(tr("Preview"));

    ui->preview->setEngine(m_Engine);

    m_MainDocument = ui->viewportWidget;

    Input::init(ui->preview);

    m_Undo = UndoManager::instance()->createUndoAction(ui->menuEdit);
    m_Undo->setShortcut(QKeySequence("Ctrl+Z"));
    ui->menuEdit->insertAction(ui->actionEditor_Mode, m_Undo);

    m_Redo = UndoManager::instance()->createRedoAction(ui->menuEdit);
    m_Redo->setShortcut(QKeySequence("Ctrl+Y"));
    ui->menuEdit->insertAction(ui->actionEditor_Mode, m_Redo);

    ui->menuEdit->insertSeparator(ui->actionEditor_Mode);

    ui->componentButton->setProperty("blue", true);
    ui->commitButton->setProperty("green", true);

    ui->hierarchy->setContextMenu(ui->viewportWidget->contextMenu());

    connect(ui->actionBuild_All, &QAction::triggered, this, &MainWindow::onBuildProject);

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
    connect(comp, SIGNAL(componentSelected(QString)), ui->viewportWidget, SIGNAL(createComponent(QString)));
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

    connect(ui->toolWidget, &QToolWindowManager::toolWindowVisibilityChanged, this, &MainWindow::onToolWindowVisibilityChanged);
    connect(ui->toolWidget, &QToolWindowManager::currentToolWindowChanged, this, &MainWindow::onCurrentToolWindowChanged);

    connect(ui->viewportWidget, &SceneComposer::hierarchyCreated, ui->hierarchy, &HierarchyBrowser::onSetRootObject, Qt::DirectConnection);
    connect(ui->viewportWidget, &SceneComposer::itemUpdated, ui->hierarchy, &HierarchyBrowser::onObjectUpdated);
    connect(ui->viewportWidget, &SceneComposer::itemsSelected, ui->hierarchy, &HierarchyBrowser::onObjectSelected);
    connect(ui->viewportWidget, &SceneComposer::itemsSelected, ui->timeline, &Timeline::onObjectsSelected);
    connect(ui->viewportWidget, &SceneComposer::itemsChanged, ui->timeline, &Timeline::onObjectsChanged);

    connect(ui->hierarchy, &HierarchyBrowser::selected, ui->viewportWidget, &SceneComposer::onSelectActors);
    connect(ui->hierarchy, &HierarchyBrowser::removed, ui->viewportWidget, &SceneComposer::onRemoveActors);
    connect(ui->hierarchy, &HierarchyBrowser::updated, ui->viewportWidget, &SceneComposer::onUpdated);
    connect(ui->hierarchy, &HierarchyBrowser::parented, ui->viewportWidget, &SceneComposer::onParentActors);
    connect(ui->hierarchy, &HierarchyBrowser::focused, ui->viewportWidget, &SceneComposer::onFocusActor);

    connect(ui->contentBrowser, &ContentBrowser::assetSelected, this, &MainWindow::onItemSelected);
    connect(ui->contentBrowser, &ContentBrowser::openEditor, this, &MainWindow::onOpenEditor);

    connect(ui->timeline, &Timeline::animated, ui->propertyView, &PropertyEditor::onAnimated);
    connect(ui->timeline, &Timeline::moved, ui->viewportWidget, &SceneComposer::onUpdated);
    connect(ui->timeline, &Timeline::objectSelected, ui->viewportWidget, &SceneComposer::onSelectActors);

    ui->toolWidget->setVisible(false);

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
        action->setChecked(false);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }

    AssetManager::ClassMap map = AssetManager::instance()->classMaps();
    if(!map.isEmpty()) {
        ui->classMapView->setModel(map.first());
    }

    connect(AssetManager::instance(), &AssetManager::buildSuccessful, ComponentModel::instance(), &ComponentModel::update);

    onItemSelected(nullptr);

    on_actionEditor_Mode_triggered();
    resetGeometry();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onItemSelected(QObject *item) {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(ui->propertyView->object());
    if(settings && settings != item) {
        AssetManager::instance()->checkImportSettings(settings);
        disconnect(settings, &AssetConverterSettings::updated, this, &MainWindow::onSettingsUpdated);
    }

    settings = dynamic_cast<AssetConverterSettings *>(item);
    if(settings) {
        connect(settings, &AssetConverterSettings::updated, this, &MainWindow::onSettingsUpdated);

        ui->commitButton->setEnabled(settings->isModified());
        ui->revertButton->setEnabled(settings->isModified());
    }

    bool isNext = (dynamic_cast<NextObject *>(item) != nullptr);
    ui->componentButton->setVisible(isNext);

    ui->commitButton->setVisible(settings);
    ui->revertButton->setVisible(settings);

    ui->propertyView->setObject(item);
}

void MainWindow::onOpenEditor(const QString &path) {
    if(m_DocumentModel == nullptr) {
        return;
    }
    AssetEditor *editor = m_DocumentModel->openFile(path);
    if(editor) {
        connect(editor, SIGNAL(updateAsset()), ui->contentBrowser, SLOT(assetUpdated()), Qt::UniqueConnection);

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
    }
}

void MainWindow::onSettingsUpdated() {
    ui->commitButton->setEnabled(true);
    ui->revertButton->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);

    if(m_DocumentModel) {
        for(auto &it : m_DocumentModel->documents()) {
            ui->toolWidget->activateToolWindow(it);
            if(!m_DocumentModel->checkSave(it)) {
                event->ignore();
                return;
            }
        }
    }

    QString str = ProjectManager::instance()->projectId();
    if(!str.isEmpty()) {
        VariantList params;
        params.push_back(qPrintable(ui->viewportWidget->path()));
        params.push_back(ui->viewportWidget->saveState());

        QSettings settings(COMPANY_NAME, EDITOR_NAME);
        settings.setValue(str, QString::fromStdString(Json::save(params)));

        SettingsManager::instance()->saveSettings();

        saveWorkspace();
    }

    QApplication::quit();
}

void MainWindow::on_commitButton_clicked() {
    AssetConverterSettings *s = dynamic_cast<AssetConverterSettings *>(ui->propertyView->object());
    if(s && s->isModified()) {
        s->saveSettings();
        AssetManager::instance()->pushToImport(s);
        AssetManager::instance()->reimport();
    }

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}

void MainWindow::on_revertButton_clicked() {
    AssetConverterSettings *s = dynamic_cast<AssetConverterSettings *>(ui->propertyView->object());
    if(s && s->isModified()) {
        s->loadSettings();
        ui->propertyView->onUpdated();
    }

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}

void MainWindow::on_actionNew_triggered() {
    m_DocumentModel->newFile(m_MainDocument);

    UndoManager::instance()->clear();
}

void MainWindow::on_actionSave_triggered() {
    if(!ui->actionGame_Mode->isChecked()) {
        m_DocumentModel->saveFile(dynamic_cast<AssetEditor *>(m_CurrentDocument));
    } else {
        QApplication::beep();
    }
}

void MainWindow::on_actionSave_As_triggered() {
    if(!ui->actionGame_Mode->isChecked()) {
        m_DocumentModel->saveFileAs(dynamic_cast<AssetEditor *>(m_CurrentDocument));
    }
}

void MainWindow::on_actionEditor_Mode_triggered() {
    setGameMode(false);
}

void MainWindow::on_actionGame_Mode_triggered() {
    setGameMode(true);
}

void MainWindow::setGameMode(bool mode) {
    if(mode) {
        if(ui->preview->parent() == nullptr) {
            ui->toolWidget->moveToolWindow(ui->preview, QToolWindowManager::ReferenceAddTo, ui->toolWidget->areaFor(ui->viewportWidget));
        }
        ui->toolWidget->activateToolWindow(ui->preview);
        ui->viewportWidget->backupScene();
        Timer::reset();
    } else {
        ui->toolWidget->activateToolWindow(ui->viewportWidget);
        ui->viewportWidget->restoreBackupScene();
    }

    ui->actionEditor_Mode->setChecked(!mode);
    ui->actionGame_Mode->setChecked(mode);

    m_Undo->setEnabled(!mode);
    m_Redo->setEnabled(!mode);

    Engine::setGameMode(mode);
}

void MainWindow::on_actionTake_Screenshot_triggered() {
    ui->viewportWidget->takeScreenshot();
}

void MainWindow::onOpenProject(const QString &path) {
    connect(m_Queue, &ImportQueue::finished, this, &MainWindow::onImportFinished, Qt::QueuedConnection);

    ui->quickWidget->setVisible(false);
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
        connect(action, &QAction::triggered, this, &MainWindow::onBuildProject);
    }

    ui->timeline->showBar();
}

void MainWindow::onNewProject() {
    QString path = QFileDialog::getSaveFileName(this, tr("Create New Project"),
                                                ProjectManager::instance()->myProjectsPath(), "*" + gProjectExt);
    if(!path.isEmpty()) {
        QFileInfo info(path);
        if(info.suffix().isEmpty()) {
            path += gProjectExt;
        }
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            file.close();
            onOpenProject(path);
        }
    }
}

void MainWindow::onImportProject() {
    QString path = QFileDialog::getOpenFileName(this, tr("Import Existing Project"),
                                                ProjectManager::instance()->myProjectsPath(), "*" + gProjectExt);
    if(!path.isEmpty()) {
        onOpenProject(path);
    }
}

void MainWindow::onImportFinished() {
    m_DocumentModel = new DocumentModel;
    connect(m_DocumentModel, &DocumentModel::itemSelected, this, &MainWindow::onItemSelected);
    connect(m_DocumentModel, &DocumentModel::itemUpdated, ui->propertyView, &PropertyEditor::onUpdated);

    ui->viewportWidget->setEngine(m_Engine);
    m_DocumentModel->addEditor(ui->viewportWidget);

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant map = settings.value(ProjectManager::instance()->projectId());
    if(map.isValid()) {
        VariantList list = Json::load(map.toString().toStdString()).toList();
        if(!list.empty()) {
            auto it = list.begin();

            string map = it->toString();
            if(map.empty()) {
                on_actionNew_triggered();
            } else {
                m_DocumentModel->openFile(it->toString().c_str());
            }
            it++;
            VariantList params = it->toList();
            if(params.size() > 3) {
                ui->viewportWidget->restoreState(params);
            }
        } else {
            on_actionNew_triggered();
        }
    } else {
        on_actionNew_triggered();
    }
    disconnect(m_Queue, nullptr, this, nullptr);

    ui->preferencesWidget->setModel(SettingsManager::instance());
    ui->projectWidget->setModel(ProjectManager::instance());

    ComponentModel::instance()->update();
    SettingsManager::instance()->loadSettings();

    ui->actionNew->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->menuEdit->setEnabled(true);
    ui->menuWindow->setEnabled(true);
    ui->menuBuild_Project->setEnabled(true);
}

void MainWindow::onWorkspaceActionClicked() {
    m_CurrentWorkspace = static_cast<QAction*>(sender())->data().toString();
    on_actionReset_Workspace_triggered();
}

void MainWindow::onToolWindowActionToggled(bool state) {
    QWidget *toolWindow = static_cast<QAction*>(sender())->data().value<QWidget *>();
    ui->toolWidget->moveToolWindow(toolWindow, state ?
                                      QToolWindowManager::NewFloatingArea :
                                      QToolWindowManager::NoArea);
}

void MainWindow::onToolWindowVisibilityChanged(QWidget *toolWindow, bool visible) {
    QAction *action = ui->menuWindow->findChild<QAction *>(toolWindow->windowTitle());
    if(action) {
        action->blockSignals(true);
        action->setChecked(visible);
        action->blockSignals(false);
    }
}

void MainWindow::on_actionSave_Workspace_triggered() {
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

void MainWindow::on_actionReset_Workspace_triggered() {
    QFile file(m_CurrentWorkspace);
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QDataStream ds(&data, QIODevice::ReadOnly);
        QVariantMap layout;
        ds >> layout;
        ui->toolWidget->restoreState(layout.value(gWindows));

        for(auto it : ui->menuWorkspace->children()) {
            QAction *action = static_cast<QAction*>(it);
            action->blockSignals(true);
            action->setChecked((action->data().toString() == m_CurrentWorkspace));
            action->blockSignals(false);
        }
    }
}

void MainWindow::saveWorkspace() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    settings.setValue(gGeometry, saveGeometry());
    settings.setValue(gWindows, ui->toolWidget->saveState());
    settings.setValue(gWorkspace, m_CurrentWorkspace);
    qDebug() << "Workspace saved";
}

void MainWindow::resetWorkspace() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QVariant windows = settings.value(gWindows);
    m_CurrentWorkspace = settings.value(gWorkspace, m_CurrentWorkspace).toString();
     ui->toolWidget->setVisible(true);
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

void MainWindow::findWorkspaces(const QString &dir) {
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

void MainWindow::resetGeometry() {
    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    restoreGeometry(settings.value(gGeometry).toByteArray());
}

void MainWindow::onBuildProject() {
    QAction *action = dynamic_cast<QAction *>(sender());
    if(action) {
        ProjectManager::instance()->build(action->property(qPrintable(gPlatforms)).toString());
    }
}

void MainWindow::onCurrentToolWindowChanged(QWidget *toolWindow) {
    AssetEditor *editor = dynamic_cast<AssetEditor *>(toolWindow);
    if(editor == m_CurrentDocument) {
        return;
    }

    if(editor) {
        m_CurrentDocument = editor;
    } else if(ui->viewportWidget == toolWindow) {
        m_CurrentDocument = m_MainDocument;
    }
    ui->hierarchy->onSetRootObject(nullptr);
    m_CurrentDocument->onActivated();
}

void MainWindow::on_menuFile_aboutToShow() {
    QString name;
    if(m_CurrentDocument && m_CurrentDocument != m_MainDocument) {
        name = QString(" \"%1\"").arg(m_DocumentModel->fileName(dynamic_cast<AssetEditor *>(m_CurrentDocument)));
    }
    ui->actionSave->setText(tr("Save%1").arg(name));
    ui->actionSave_As->setText(tr("Save%1 As...").arg(name));
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void MainWindow::on_actionReport_Issue_triggered() {
    QDesktopServices::openUrl(QUrl("https://github.com/thunder-engine/thunder/issues/new/choose", QUrl::TolerantMode));
}

void MainWindow::on_actionAPI_Reference_triggered() {
    QDesktopServices::openUrl(QUrl("https://doc.thunderengine.org/en/latest/reference/index.html", QUrl::TolerantMode));
}

void MainWindow::on_actionThunder_Answers_triggered() {
    QDesktopServices::openUrl(QUrl("https://github.com/thunder-engine/thunder/discussions", QUrl::TolerantMode));
}

void MainWindow::on_actionThunder_Manual_triggered() {
    QDesktopServices::openUrl(QUrl("https://doc.thunderengine.org/en/latest", QUrl::TolerantMode));
}
