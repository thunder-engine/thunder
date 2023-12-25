#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>

#include <QQmlContext>
#include <QQuickItem>

#include <json.h>
#include <timer.h>
#include <log.h>

#include <cstring>

#include <editor/asseteditor.h>

// Misc
#include "managers/assetimporter/importqueue.h"
#include "managers/feedmanager/feedmanager.h"
#include "managers/projectmanager/projectmodel.h"

#include <editor/assetmanager.h>
#include <editor/projectmanager.h>
#include <editor/undomanager.h>
#include <editor/pluginmanager.h>
#include <editor/settingsmanager.h>
#include <editor/editorgadget.h>

#include "documentmodel.h"

// System
#include <global.h>
#include "config.h"

#include "screens/componentbrowser/componentmodel.h"
#include "screens/propertyedit/propertyeditor.h"
#include "screens/objecthierarchy/hierarchybrowser.h"
#include "screens/scenecomposer/scenecomposer.h"
#include "screens/preview/preview.h"

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
        m_currentWorkspace(":/Workspaces/Default.ws"),
        m_engine(engine),
        m_queue(new ImportQueue),
        m_projectModel(new ProjectModel),
        m_feedManager(new FeedManager),
        m_documentModel(nullptr),
        m_undo(nullptr),
        m_redo(nullptr),
        m_preview(nullptr),
        m_mainEditor(nullptr),
        m_currentEditor(nullptr),
        m_builder(new QProcess(this)),
        m_forceReimport(false) {

    qRegisterMetaType<Vector2>  ("Vector2");
    qRegisterMetaType<Vector3>  ("Vector3");

    qRegisterMetaType<uint8_t>  ("uint8_t");
    qRegisterMetaType<uint32_t> ("uint32_t");

    qmlRegisterType<ProjectModel>("com.frostspear.thunderengine", 1, 0, "ProjectModel");

    SettingsManager::instance()->registerProperty("General/Language", QLocale());

    ui->setupUi(this);

    ui->playButton->setProperty("checkblue", true);
    ui->pauseButton->setProperty("checkblue", true);

    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::on_actionPlay_triggered);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MainWindow::on_actionPause_triggered);

    ui->projectWidget->setWindowTitle(tr("Project Settings"));
    ui->projectWidget->setWindowIcon(QIcon(":/Style/styles/dark/icons/gear.png"));

    ui->preferencesWidget->setWindowTitle(tr("Editor Preferences"));
    ui->preferencesWidget->setWindowIcon(QIcon(":/Style/styles/dark/icons/equalizer.png"));

    m_undo = UndoManager::instance()->createUndoAction(ui->menuEdit);
    m_undo->setShortcut(QKeySequence("Ctrl+Z"));
    ui->menuEdit->insertAction(ui->actionPlay, m_undo);

    m_redo = UndoManager::instance()->createRedoAction(ui->menuEdit);
    m_redo->setShortcut(QKeySequence("Ctrl+Y"));
    ui->menuEdit->insertAction(ui->actionPlay, m_redo);

    ui->menuEdit->insertSeparator(ui->actionPlay);

    connect(ui->actionBuild_All, &QAction::triggered, this, &MainWindow::onBuildProject);

    ui->quickWidget->rootContext()->setContextProperty("projectsModel", m_projectModel);
    ui->quickWidget->rootContext()->setContextProperty("feedManager", m_feedManager);
    ui->quickWidget->setSource(QUrl("qrc:/QML/qml/Startup.qml"));
    QQuickItem *item = ui->quickWidget->rootObject();
    connect(item, SIGNAL(openProject(QString)), this, SLOT(onOpenProject(QString)));
    connect(item, SIGNAL(newProject()), this, SLOT(onNewProject()));
    connect(item, SIGNAL(importProject()), this, SLOT(onImportProject()));

    findWorkspaces(":/Workspaces");
    findWorkspaces("workspaces");
    ui->menuWorkspace->insertSeparator(ui->actionReset_Workspace);

    ui->actionAbout->setText(tr("About %1...").arg(EDITOR_NAME));
    connect(ui->actionAbout, &QAction::triggered, &m_aboutDlg, &AboutDialog::exec);
    connect(ui->actionPlugin_Manager, &QAction::triggered, &m_pluginDlg, &PluginDialog::exec);

    connect(ui->toolWidget, &QToolWindowManager::toolWindowVisibilityChanged, this, &MainWindow::onToolWindowVisibilityChanged);
    connect(ui->toolWidget, &QToolWindowManager::currentToolWindowChanged, this, &MainWindow::onCurrentToolWindowChanged);

    connect(ui->contentBrowser, &ContentBrowser::openEditor, this, &MainWindow::onOpenEditor);

    ui->toolPanel->setVisible(false);
    ui->toolWidget->setVisible(false);

    ui->toolWidget->addToolWindow(ui->contentBrowser,    QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->consoleOutput,     QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->projectWidget,     QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(ui->preferencesWidget, QToolWindowManager::NoArea);

    connect(AssetManager::instance(), &AssetManager::buildSuccessful, ComponentModel::instance(), &ComponentModel::update);

    resetGeometry();

    connect(m_builder, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int,QProcess::ExitStatus)));

    connect(m_builder, &QProcess::readyReadStandardOutput, this, &MainWindow::readOutput);
    connect(m_builder, &QProcess::readyReadStandardError, this, &MainWindow::readError);

    connect(this, &MainWindow::readBuildLogs, ui->consoleOutput, &ConsoleManager::parseLogs);

    startTimer(16);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addGadget(EditorGadget *gadget) {
    m_gadgets.push_back(gadget);

    ui->toolWidget->addToolWindow(gadget, QToolWindowManager::NoArea);

    connect(m_documentModel, &DocumentModel::updated, gadget, &EditorGadget::onUpdated);
    connect(m_documentModel, &DocumentModel::itemsSelected, gadget, &EditorGadget::onItemsSelected);
    connect(m_documentModel, &DocumentModel::objectsSelected, gadget, &EditorGadget::onObjectsSelected);

    connect(ui->contentBrowser, &ContentBrowser::assetsSelected, gadget, &EditorGadget::onItemsSelected);
}

AssetEditor *MainWindow::openEditor(const QString &path) {
    if(m_documentModel == nullptr) {
        return nullptr;
    }
    AssetEditor *editor = m_documentModel->openFile(path);
    if(editor) {
        if(ui->toolWidget->areaFor(editor) == nullptr) {
            QWidget *neighbor = m_mainEditor;
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

    return editor;
}

void MainWindow::onOpenEditor(const QString &path) {
    openEditor(path);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);

    QString str = ProjectManager::instance()->projectId();
    if(!str.isEmpty()) {
        QSettings settings(COMPANY_NAME, EDITOR_NAME);

        // Save status for open editors
        if(m_documentModel) {
            VariantList editors;

            for(auto &it : m_documentModel->documents()) {
                VariantList documents;
                for(auto &doc : it->openedDocuments()) {
                    VariantList fields = {qPrintable(doc->source())};
                    documents.push_back(fields);
                }

                VariantList params = {documents, it->saveState()};

                editors.push_back(params);

                ui->toolWidget->activateToolWindow(it);
                if(!it->checkSave()) {
                    event->ignore();
                    return;
                }
            }

            settings.setValue(str, QString::fromStdString(Json::save(editors)));
        }

        // Save editor preferences
        SettingsManager::instance()->saveSettings();

        // Save workspace
        settings.setValue(gGeometry, saveGeometry());
        settings.setValue(gWindows, ui->toolWidget->saveState());
        settings.setValue(gWorkspace, m_currentWorkspace);
    }

    QApplication::quit();
}

void MainWindow::on_actionNew_triggered() {
    m_documentModel->newFile(m_mainEditor);

    UndoManager::instance()->clear();
}

void MainWindow::on_actionOpen_triggered() {
    QString path = QFileDialog::getOpenFileName(this, tr("Open Scene"),
                                                ProjectManager::instance()->contentPath(), "*.map");
    if(!path.isEmpty()) {
        m_documentModel->openFile(path);
    }
}

void MainWindow::on_actionSave_triggered() {
    if(!Engine::isGameMode()) {
        m_currentEditor->onSave();
    } else {
        QApplication::beep();
    }
}

void MainWindow::on_actionSave_As_triggered() {
    if(!Engine::isGameMode()) {
        m_currentEditor->onSaveAs();
    }
}

void MainWindow::on_actionPlay_triggered() {
    setGameMode(!Engine::isGameMode());
}

void MainWindow::on_actionPause_triggered() {
    if(m_preview) {
        m_preview->setGamePause(!m_preview->isGamePause());
        ui->pauseButton->setChecked(m_preview->isGamePause());
    }
}

void MainWindow::setGameMode(bool mode) {
    if(m_preview) {
        if(mode) {
            if(m_preview->parent() == nullptr) {
                ui->toolWidget->moveToolWindow(m_preview, QToolWindowManager::ReferenceAddTo, ui->toolWidget->areaFor(m_mainEditor));
            }
            static_cast<SceneComposer *>(m_mainEditor)->backupScenes();
            ui->toolWidget->activateToolWindow(m_preview);
            m_preview->onActivate();
        } else {
            ui->toolWidget->activateToolWindow(m_mainEditor);
            static_cast<SceneComposer *>(m_mainEditor)->restoreBackupScenes();

            m_preview->setGamePause(false);
            ui->pauseButton->setChecked(false);
            ui->actionPause->setChecked(false);
        }
    }

    ui->playButton->setChecked(mode);
    ui->actionPlay->setChecked(mode);

    m_undo->setEnabled(!mode);
    m_redo->setEnabled(!mode);

    Engine::setGameMode(mode);
}

void MainWindow::onOpenProject(const QString &path) {
    connect(m_queue, &ImportQueue::importFinished, this, &MainWindow::onImportFinished, Qt::QueuedConnection);

    ui->quickWidget->setVisible(false);

    m_projectModel->addProject(path);
    ProjectManager::instance()->init(path);

    PluginManager::instance()->init(m_engine);
    AssetManager::instance()->init();

    ProjectManager::instance()->loadPlatforms();

    // Read settings early for converters
    SettingsManager::instance()->loadSettings();

    m_forceReimport = false;
    QString projectSDK = ProjectManager::instance()->projectSdk();
    if(projectSDK != SDK_VERSION) {
        m_forceReimport = true;
    }

    Engine::file()->fsearchPathAdd(qPrintable(ProjectManager::instance()->importPath()), true);

    if(!PluginManager::instance()->rescanProject(ProjectManager::instance()->pluginsPath())) {
        AssetManager::instance()->rebuild();
    }

    m_forceReimport |= !Engine::reloadBundle();

    PluginManager::instance()->initSystems();

    AssetManager::instance()->rescan(m_forceReimport);

    for(QString &it : ProjectManager::instance()->platforms()) {
        QString name = it;
        name.replace(0, 1, name.at(0).toUpper());
        QAction *action = ui->menuBuild_Project->addAction(tr("Build for %1").arg(name));
        action->setProperty(qPrintable(gPlatforms), it);
        connect(action, &QAction::triggered, this, &MainWindow::onBuildProject);
    }
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
    m_documentModel = new DocumentModel;

    m_mainEditor = new SceneComposer(this);
    m_documentModel->addEditor(m_mainEditor);

    m_preview = new Preview(this);

    addGadget(new PropertyEditor(this));
    addGadget(new HierarchyBrowser(this));
    for(auto &it : PluginManager::instance()->extensions("gadget")) {
        addGadget(reinterpret_cast<EditorGadget *>(PluginManager::instance()->getPluginObject(it)));
    }

    ui->toolWidget->addToolWindow(m_preview,    QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(m_mainEditor, QToolWindowManager::NoArea);

    foreach(QWidget *it, ui->toolWidget->toolWindows()) {
        QAction *action = new QAction(it->windowTitle(), ui->menuWindow);
        ui->menuWindow->addAction(action);
        action->setObjectName(it->windowTitle());
        action->setData(QVariant::fromValue(it));
        action->setCheckable(true);
        action->setChecked(false);

        connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    }

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    // Load last session state
    QVariant windows = settings.value(gWindows);
    m_currentWorkspace = settings.value(gWorkspace, m_currentWorkspace).toString();
    ui->toolPanel->setVisible(true);
    ui->toolWidget->setVisible(true);
    if(!windows.isValid() || !ui->toolWidget->restoreState(windows)) {
        on_actionReset_Workspace_triggered();
    } else {
        for(auto it : ui->menuWorkspace->children()) {
            QAction *action = static_cast<QAction*>(it);
            action->blockSignals(true);
            action->setChecked((action->data().toString() == m_currentWorkspace));
            action->blockSignals(false);
        }
    }
    // Open the same editors with documents from the last session
    QVariant map = settings.value(ProjectManager::instance()->projectId());
    if(map.isValid()) {
        VariantList editors = Json::load(map.toString().toStdString()).toList();
        if(!editors.empty()) {
            for(auto &it : editors) {
                VariantList params = it.toList();
                if(params.size() == 2) {
                    AssetEditor *editor = nullptr;
                    // Documents
                    for(auto &document : params.front().toList()) {
                        AssetEditor *e = openEditor(document.toList().front().toString().c_str());
                        if(e) {
                            editor = e;
                        }
                    }

                    if(editor) {
                        editor->restoreState(params.back().toList());
                    }
                }
            }
        }
    }
    // Set ui state
    disconnect(m_queue, nullptr, this, nullptr);

    ComponentModel::instance()->update();
    SettingsManager::instance()->loadSettings();

    ui->projectWidget->onItemsSelected({ProjectManager::instance()});
    ui->preferencesWidget->onItemsSelected({SettingsManager::instance()});

    ui->actionNew->setEnabled(true);
    ui->actionOpen->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->menuEdit->setEnabled(true);
    ui->menuWindow->setEnabled(true);
    ui->menuBuild_Project->setEnabled(true);

    if(m_forceReimport) {
        ProjectManager::instance()->setProjectSdk(SDK_VERSION);
        ProjectManager::instance()->saveSettings();
    }

    setGameMode(false);
}

void MainWindow::onWorkspaceActionClicked() {
    m_currentWorkspace = static_cast<QAction*>(sender())->data().toString();
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
    QFile file(m_currentWorkspace);
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
            action->setChecked((action->data().toString() == m_currentWorkspace));
            action->blockSignals(false);
        }
    }
}

void MainWindow::findWorkspaces(const QString &dir) {
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while(it.hasNext()) {
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
        build(action->property(qPrintable(gPlatforms)).toString());
    }
}

void MainWindow::onCurrentToolWindowChanged(QWidget *toolWindow) {
    AssetEditor *editor = dynamic_cast<AssetEditor *>(toolWindow);
    if(editor == m_currentEditor) {
        return;
    }

    if(editor) {
        foreach(auto it, m_gadgets) {
            if(m_currentEditor) {
                disconnect(it, &EditorGadget::updated, m_currentEditor, &AssetEditor::onUpdated);
                disconnect(it, &EditorGadget::objectsSelected, m_currentEditor, &AssetEditor::onObjectsSelected);

                disconnect(m_currentEditor, &AssetEditor::objectsChanged, it, &EditorGadget::onObjectsChanged);
            }

            connect(it, &EditorGadget::updated, editor, &AssetEditor::onUpdated);
            connect(it, &EditorGadget::objectsSelected, editor, &AssetEditor::onObjectsSelected);

            connect(editor, &AssetEditor::objectsChanged, it, &EditorGadget::onObjectsChanged);

            it->setCurrentEditor(editor);
        }

        m_currentEditor = editor;
        m_currentEditor->onActivated();
    }
}

void MainWindow::on_menuFile_aboutToShow() {
    QString name;
    if(m_currentEditor && m_currentEditor != m_mainEditor) {
        AssetConverterSettings *settings = m_currentEditor->openedDocuments().first();
        name = QString(" \"%1\"").arg(settings->source());
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

void MainWindow::on_actionExit_triggered() {
    closeEvent(new QCloseEvent);
}

void MainWindow::build(QString platform) {
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Select Target Directory"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()) {
        ProjectManager *mgr = ProjectManager::instance();

        QStringList args;
        args << "-s" << mgr->projectPath() << "-t" << dir;

        if(!platform.isEmpty()) {
            args << "-p" << platform;
        }

        qDebug() << args.join(" ");

        m_builder->start("Builder", args);
        if(!m_builder->waitForStarted()) {
            aError() << qPrintable(m_builder->errorString());
        }
    }
}

void MainWindow::onBuildFinished(int exitCode, QProcess::ExitStatus) {
    QMessageBox msg;
    if(exitCode == 0) {
        msg.setText("Build Succeeded.");
        msg.setIcon(QMessageBox::Information);
        aInfo() << qPrintable(msg.text());
    } else {
        msg.setText("Build Failed. Please check log output for more details.");
        msg.setIcon(QMessageBox::Critical);
        aError() << qPrintable(msg.text());
    }
    msg.exec();
}

void MainWindow::readOutput() {
    emit readBuildLogs(m_builder->readAllStandardOutput());
}

void MainWindow::readError() {
    emit readBuildLogs(m_builder->readAllStandardError());
}

void MainWindow::timerEvent(QTimerEvent *) {
    Timer::update();
}
