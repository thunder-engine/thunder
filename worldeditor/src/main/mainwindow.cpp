#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QSettings>
#include <QApplication>
#include <QMessageBox>

#include <json.h>
#include <timer.h>
#include <log.h>
#include <url.h>
#include <filedialog.h>

#include <editor/asseteditor.h>

// Misc
#include "managers/assetimporter/importqueue.h"
#include "managers/projectmanager/projectmodel.h"

#include <editor/assetmanager.h>
#include <editor/projectsettings.h>
#include <editor/undostack.h>
#include <editor/pluginmanager.h>
#include <editor/editorsettings.h>
#include <editor/editorgadget.h>
#include <editor/nativecodebuilder.h>

// System
#include <global.h>
#include "config.h"

#include "screens/componentbrowser/componentmodel.h"
#include "screens/contentbrowser/contentbrowser.h"
#include "screens/consoleoutput/consolemanager.h"
#include "screens/propertyedit/propertyeditor.h"
#include "screens/objecthierarchy/hierarchybrowser.h"
#include "screens/projectsettings/projectsettingsbrowser.h"
#include "screens/editorsettings/editorsettingsbrowser.h"
#include "screens/scenecomposer/scenecomposer.h"
#include "screens/preview/preview.h"

Q_DECLARE_METATYPE(Object *)
Q_DECLARE_METATYPE(Object::ObjectList *)

namespace  {
    const char *gGeometry("main.geometry");
    const char *gWindows("main.windows");
    const char *gWorkspace("main.workspace");
};

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        m_currentWorkspace(":/Workspaces/Default.ws"),
        m_queue(nullptr),
        m_editorSettingsBrowser(new EditorSettingsBrowser(this)),
        m_projectSettingsBrowser(new ProjectSettingsBrowser(this)),
        m_contentBrowser(new ContentBrowser(this)),
        m_consoleOutput(new ConsoleManager(this)),
        m_preview(nullptr),
        m_builder(new QProcess(this)) {

    qRegisterMetaType<Vector2>  ("Vector2");
    qRegisterMetaType<Vector3>  ("Vector3");

    qRegisterMetaType<uint8_t>  ("uint8_t");
    qRegisterMetaType<uint32_t> ("uint32_t");

    QLocale locale(QLocale::English, QLocale::UnitedStates);
    Editor::settings()->registerValue("General/Language", TString(locale.bcp47Name().toStdString()), "editor=Locale");

    ui->setupUi(this);

    ui->playButton->setProperty("checkblue", true);
    ui->pauseButton->setProperty("checkblue", true);

    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::on_actionPlay_triggered);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MainWindow::on_actionPause_triggered);

    connect(ui->actionBuild_All, &QAction::triggered, this, &MainWindow::onBuildProject);

    findWorkspaces(":/Workspaces");
    findWorkspaces("workspaces");
    ui->menuWorkspace->insertSeparator(ui->actionReset_Workspace);

    ui->actionAbout->setText(tr("About %1...").arg(EDITOR_NAME));
    connect(ui->actionAbout, &QAction::triggered, &m_aboutDlg, &AboutDialog::exec);
    connect(ui->actionPlugin_Manager, &QAction::triggered, &m_pluginDlg, &PluginDialog::exec);

    connect(ui->toolWidget, &QToolWindowManager::toolWindowVisibilityChanged, this, &MainWindow::onToolWindowVisibilityChanged);
    connect(ui->toolWidget, &QToolWindowManager::currentToolWindowChanged, this, &MainWindow::onCurrentToolWindowChanged);

    connect(m_contentBrowser, &ContentBrowser::openEditor, this, &MainWindow::onOpenEditor);

    ui->toolPanel->setVisible(false);
    ui->toolWidget->setVisible(false);

    ui->toolWidget->addToolWindow(m_contentBrowser, QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(m_consoleOutput, QToolWindowManager::NoArea);

    ui->toolWidget->addToolWindow(m_projectSettingsBrowser, QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(m_editorSettingsBrowser, QToolWindowManager::NoArea);

    resetGeometry();

    connect(m_builder, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int,QProcess::ExitStatus)));

    connect(m_builder, &QProcess::readyReadStandardOutput, this, &MainWindow::readOutput);
    connect(m_builder, &QProcess::readyReadStandardError, this, &MainWindow::readError);

    connect(this, &MainWindow::readBuildLogs, m_consoleOutput, &ConsoleManager::parseLogs);

    startTimer(16);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::addGadget(EditorGadget *gadget) {
    ui->toolWidget->addToolWindow(gadget, QToolWindowManager::NoArea);

    Editor::addGadget(gadget);
}

AssetEditor *MainWindow::openEditor(const TString &path) {
    AssetEditor *editor = Editor::openFile(path);
    if(editor) {
        if(ui->toolWidget->areaFor(editor) == nullptr) {
            ui->toolWidget->removeToolWindow(editor);
            editor->setParent(this);
            ui->toolWidget->addToolWindow(editor, QToolWindowManager::ReferenceAddTo, ui->toolWidget->areaFor(Editor::currentEditor()));
        } else {
            ui->toolWidget->activateToolWindow(editor);
        }
    }

    return editor;
}

void MainWindow::onOpenEditor(const TString &path) {
    openEditor(path);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);

    TString str = Editor::project()->projectId();
    if(!str.isEmpty()) {
        QSettings settings(COMPANY_NAME, EDITOR_NAME);

        // Save status for open editors
        VariantList editors;

        for(auto &it : Editor::documents()) {
            VariantMap editorState = it->saveState();

            VariantList documents;
            for(auto &doc : it->openedDocuments()) {
                documents.push_back({doc->source()});
            }

            VariantList params = {documents, editorState};

            editors.push_back(params);

            ui->toolWidget->activateToolWindow(it);
            if(!it->checkSave()) {
                event->ignore();
                return;
            }
        }

        settings.setValue(str.data(), Json::save(editors).data());

        // Save workspace
        settings.setValue(gGeometry, saveGeometry());
        settings.setValue(gWindows, ui->toolWidget->saveState());
        settings.setValue(gWorkspace, m_currentWorkspace);
    }

    QApplication::quit();
}

void MainWindow::on_actionNew_triggered() {
    AssetEditor *editor = Editor::currentEditor();
    if(editor && editor->checkSave()) {
        Editor::closeEditor(editor);
        editor->onNewAsset();
    }
}

void MainWindow::on_actionOpen_triggered() {
    AssetEditor *editor = Editor::currentEditor();
    if(editor && editor->checkSave()) {
        Editor::closeEditor(editor);
        editor->onOpenAsset();
    }
}

void MainWindow::on_actionSave_triggered() {
    if(!Engine::isGameMode()) {
        Editor::currentEditor()->onSave();
    } else {
        QApplication::beep();
    }
}

void MainWindow::on_actionSave_As_triggered() {
    if(!Engine::isGameMode()) {
        AssetEditor *editor = Editor::currentEditor();
        if(editor && editor->allowSaveAs()) {
            editor->onSaveAs();
        }
    }
}

void MainWindow::on_actionUndo_triggered() {
    if(!Engine::isGameMode()) {
        AssetEditor *editor = Editor::currentEditor();
        if(editor) {
            editor->undoRedo()->undo();
        }
    }
}

void MainWindow::on_actionRedo_triggered() {
    if(!Engine::isGameMode()) {
        AssetEditor *editor = Editor::currentEditor();
        if(editor) {
            editor->undoRedo()->redo();
        }
    }
}

void MainWindow::on_actionPlay_triggered() {
    setGameMode(!Engine::isGameMode());
}

void MainWindow::on_actionPause_triggered() {
    if(m_preview) {
        bool pause = !m_preview->isPaused();
        m_preview->setPaused(pause);
        ui->pauseButton->setChecked(pause);
    }
}

void MainWindow::setGameMode(bool mode) {
    if(m_preview) {
        if(mode) {
            if(m_preview->parent() == nullptr) {
                ui->toolWidget->moveToolWindow(m_preview, QToolWindowManager::ReferenceAddTo, ui->toolWidget->areaFor(Editor::currentEditor()));
            }
            Editor::backup();
            ui->toolWidget->activateToolWindow(m_preview);
            m_preview->onActivate();
        } else {
            ui->toolWidget->activateToolWindow(Editor::currentEditor());
            Editor::restore();

            m_preview->setPaused(false);
            ui->pauseButton->setChecked(false);
            ui->actionPause->setChecked(false);
        }
    }

    ui->playButton->setChecked(mode);
    ui->actionPlay->setChecked(mode);

    ui->actionUndo->setEnabled(!mode);
    ui->actionRedo->setEnabled(!mode);

    Engine::setGameMode(mode);
}

void MainWindow::onOpenProject(const TString &path, Engine &engine) {
    ProjectModel::addProject(path);
    Editor::project()->init(path);

    PluginManager::instance()->init(&engine);

    m_queue = new ImportQueue;
    connect(m_queue, &ImportQueue::importFinished, this, &MainWindow::onImportFinished, Qt::QueuedConnection);

    AssetManager::instance()->init();

    Editor::project()->loadPlatforms();
    // Read settings early for converters
    Editor::settings()->loadSettings();

    if(!PluginManager::instance()->rescanProject(Editor::project()->pluginsPath())) {
        Editor::project()->currentBuilder(Editor::project()->currentPlatformName())->makeOutdated();
    }

    AssetManager::instance()->rescan();

    for(const TString &it : Editor::project()->platforms()) {
        QString name = it.data();
        name.replace(0, 1, name.at(0).toUpper());
        QAction *action = ui->menuBuild_Project->addAction(tr("Build for %1").arg(name));
        action->setProperty(gPlatforms, it.data());
        connect(action, &QAction::triggered, this, &MainWindow::onBuildProject);
    }

    m_contentBrowser->createContextMenus();
}

void MainWindow::onImportFinished() {
    PluginManager::instance()->initSystems();

    Editor::init();

    SceneComposer *composer = new SceneComposer(this);
    Editor::addEditor(composer);

    m_preview = new Preview(this);

    Editor::settings()->loadSettings();

    m_editorSettingsBrowser->init();
    m_projectSettingsBrowser->init();

    PropertyEditor *property = new PropertyEditor(this);
    connect(m_contentBrowser, &ContentBrowser::assetsSelected, property, &PropertyEditor::onObjectSelected);
    addGadget(property);
    addGadget(new HierarchyBrowser(this));

    for(auto &it : PluginManager::instance()->extensions("gadget")) {
        addGadget(reinterpret_cast<EditorGadget *>(PluginManager::instance()->getPluginObject(it)));
    }

    ui->toolWidget->addToolWindow(m_preview, QToolWindowManager::NoArea);
    ui->toolWidget->addToolWindow(composer, QToolWindowManager::NoArea);

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
    m_currentWorkspace = settings.value(gWorkspace, m_currentWorkspace).toString();
    ui->toolPanel->setVisible(true);
    ui->toolWidget->setVisible(true);
    QVariant windows = settings.value(gWindows);
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
    QVariant map = settings.value(Editor::project()->projectId().data());
    if(map.isValid()) {
        VariantList editors = Json::load(map.toString().toStdString()).toList();
        if(!editors.empty()) {
            for(auto &it : editors) {
                VariantList params = it.toList();
                if(params.size() == 2) {
                    AssetEditor *editor = nullptr;
                    // Documents
                    for(auto &document : params.front().toList()) {
                        AssetEditor *e = openEditor(document.toList().front().toString());
                        if(e) {
                            editor = e;
                        }
                    }

                    if(editor) {
                        editor->restoreState(params.back().toMap());
                    }
                }
            }
        }
    }

    if(composer->openedDocuments().empty()) {
        AssetManager *mgr = AssetManager::instance();
        TString firstMap = mgr->uuidToPath(Editor::project()->firstMap());

        AssetConverterSettings *mapSettings = mgr->fetchSettings(firstMap);
        if(mapSettings) {
            openEditor(firstMap);
        } else {
            static_cast<AssetEditor *>(composer)->onNewAsset();
        }
    }

    // Set ui state
    disconnect(m_queue, nullptr, this, nullptr);

    ComponentModel::instance()->update();

    ui->actionNew->setEnabled(true);
    ui->actionOpen->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->menuEdit->setEnabled(true);
    ui->menuWindow->setEnabled(true);
    ui->menuBuild_Project->setEnabled(true);

    PluginManager::instance()->syncWhiteList();

    TString version(SDK_VERSION);
    if(Editor::project()->projectSdk() != version) {
        Editor::project()->setProjectSdk(version);
        Editor::project()->saveSettings();
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
    FileDialog dialog;
    dialog.setDirectory(Editor::project()->templatePath() + "/workspaces");
    dialog.setWindowTitle("Save Workspace");
    dialog.setMode(FileDialog::SaveFile);
    dialog.addFilter("Workspaces", { "*.ws" });

    if(dialog.exec()) {
        TString path = dialog.getSelectedFile();
        if(!path.isEmpty()) {
            QFile file(path.data());
            if(file.open(QFile::WriteOnly)) {
                QVariantMap layout;
                layout[gWindows] = ui->toolWidget->saveState();

                QByteArray data;
                QDataStream ds(&data, QFile::WriteOnly);
                ds << layout;

                file.write(data);
                file.close();
            }
        }
    }
}

void MainWindow::on_actionReset_Workspace_triggered() {
    QFile file(m_currentWorkspace);
    if(file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QDataStream ds(&data, QFile::ReadOnly);
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
        TString path(it.next().toStdString());
        TString baseName(Url(path).baseName());

        if(!baseName.isEmpty()) {
            QAction *action = new QAction(baseName.data(), ui->menuWorkspace);
            action->setCheckable(true);
            action->setData(path.data());
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
        build(action->property(gPlatforms).toString());
    }
}

void MainWindow::onCurrentToolWindowChanged(QWidget *toolWindow) {
    AssetEditor *editor = dynamic_cast<AssetEditor *>(toolWindow);
    if(editor == Editor::currentEditor()) {
        return;
    }

    if(editor) {
        AssetEditor *currentEditor = Editor::currentEditor();
        if(currentEditor) {
            disconnect(ui->actionCut, &QAction::triggered, currentEditor, &AssetEditor::onCutAction);
            disconnect(ui->actionCopy, &QAction::triggered, currentEditor, &AssetEditor::onCopyAction);
            disconnect(ui->actionPaste, &QAction::triggered, currentEditor, &AssetEditor::onPasteAction);

            disconnect(currentEditor, &AssetEditor::copyPasteChanged, this, &MainWindow::onCopyPasteChanged);
        }

        Editor::setCurrentEditor(editor);
        if(editor) {
            editor->onActivated();

            ui->actionCut->setEnabled(editor->isCopyActionAvailable());
            ui->actionCopy->setEnabled(editor->isCopyActionAvailable());
            ui->actionPaste->setEnabled(editor->isPasteActionAvailable());

            connect(ui->actionCut, &QAction::triggered, editor, &AssetEditor::onCutAction);
            connect(ui->actionCopy, &QAction::triggered, editor, &AssetEditor::onCopyAction);
            connect(ui->actionPaste, &QAction::triggered, editor, &AssetEditor::onPasteAction);

            connect(editor, &AssetEditor::copyPasteChanged, this, &MainWindow::onCopyPasteChanged);
        }
    } else {
        Preview *preview = dynamic_cast<Preview *>(toolWindow);
        if(preview) {
            preview->onActivate();
        }
    }
}

void MainWindow::onCopyPasteChanged() {
    AssetEditor *editor = Editor::currentEditor();
    ui->actionCut->setEnabled(editor->isCopyActionAvailable());
    ui->actionCopy->setEnabled(editor->isCopyActionAvailable());
    ui->actionPaste->setEnabled(editor->isPasteActionAvailable());
}

void MainWindow::on_menuFile_aboutToShow() {
    TString name;
    TString type;
    AssetEditor *editor = Editor::currentEditor();
    if(editor) {
        type = TString(" %1").arg(editor->assetType());
        if(!editor->openedDocuments().empty()) {
            AssetConverterSettings *settings = editor->openedDocuments().front();
            name = TString(" \"%1\"").arg(settings->source());
        }
        if(name.isEmpty()) {
            name = type;
        }

        ui->actionSave_As->setEnabled(editor->allowSaveAs());
    }
    ui->actionNew->setText(tr("New%1").arg(type.data()));
    ui->actionOpen->setText(tr("Open%1").arg(type.data()));
    ui->actionSave->setText(tr("Save%1").arg(name.data()));
    ui->actionSave_As->setText(tr("Save%1 As...").arg(name.data()));
}

void MainWindow::on_menuEdit_aboutToShow() {
    UndoStack *stack = Editor::currentEditor()->undoRedo();
    ui->actionUndo->setText(tr("Undo %1").arg(stack->undoText().data()));

    ui->actionRedo->setText(tr("Redo %1").arg(stack->redoText().data()));
}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void MainWindow::on_actionReport_Issue_triggered() {
    Process::openUrl("https://github.com/thunder-engine/thunder/issues/new/choose");
}

void MainWindow::on_actionAPI_Reference_triggered() {
    Process::openUrl("https://doc.thunderengine.org/en/latest/reference/index.html");
}

void MainWindow::on_actionThunder_Answers_triggered() {
    Process::openUrl("https://github.com/thunder-engine/thunder/discussions");
}

void MainWindow::on_actionThunder_Manual_triggered() {
    Process::openUrl("https://doc.thunderengine.org/en/latest");
}

void MainWindow::on_actionExit_triggered() {
    closeEvent(new QCloseEvent);
}

void MainWindow::build(QString platform) {
    FileDialog dialog;
    dialog.setDirectory("");
    dialog.setWindowTitle("Select Target Directory");
    dialog.setMode(FileDialog::OpenDirectory);

    if(dialog.exec()) {
        TString dir = dialog.getSelectedFile();
        if(!dir.isEmpty()) {
            QStringList args;
            args << "-s" << Editor::project()->projectPath().data() << "-t" << dir.data();

            if(!platform.isEmpty()) {
                args << "-p" << platform;
            }

            m_builder->start("Builder", args);
            if(!m_builder->waitForStarted()) {
                aError() << qPrintable(m_builder->errorString());
            }
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
    if(!Engine::isGameMode() || (m_preview && m_preview->isPaused())) {
        Timer::update();
    }
}
