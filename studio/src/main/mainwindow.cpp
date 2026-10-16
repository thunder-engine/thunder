#include "mainwindow.h"
#include "viewport/viewport.h"
#include "scenecomposer/scenecomposer.h"
#include "scenecomposer/tools/spline/splinepanel.h"

#include <components/menubar.h>
#include <engine.h>
#include <pipelinecontext.h>
#include <pipelinetask.h>
#include <world.h>
#include <log.h>
#include <timer.h>

#include <canvas.h>
#include <frame.h>

#include <editor.h>
#include <editor/asseteditor.h>
#include <editor/projectsettings.h>
#include <editor/pluginmanager.h>
#include <editor/assetmanager.h>
#include <editor/editorsettings.h>
#include <editor/nativecodebuilder.h>

#include <systems/rendersystem.h>

#include "managers/assetimporter/importqueue.h"
#include "managers/toolwindowmanager/abstracttoolwindowmanagerarea.h"
#include "managers/toolwindowmanager/toolwindowmanager.h"
#include "managers/toolwindowmanager/private/toolwindowmanagerarea_p.h"
#include "screens/objecthierarchy/hierarchybrowser.h"
#include "screens/contentbrowser/contentbrowser.h"

namespace {
    const float kStatusBarHeight = 24.0f;
    const float kMenuBarHeight = 28.0f;
}

MainWindow::MainWindow() :
        m_scene(nullptr),
        m_canvas(nullptr),
        m_toolWindowManager(nullptr),
        m_menuBarWidget(nullptr),
        m_windowMenu(nullptr),
        m_hierarchyBrowser(nullptr),
        m_currentAssetEditor(nullptr),
        m_queue(nullptr) {

    m_world = dynamic_cast<World *>(Engine::objectCreate("World", "MainWorld"));
    m_scene = dynamic_cast<Scene *>(Engine::objectCreate("Scene", "MainScene", m_world));
}

MainWindow::~MainWindow() {
    setFileDropTarget(nullptr);
    if(m_menuBarWidget) {
        m_menuBarWidget->detachFromWindow();
    }
    setMenuBar(nullptr);
}

bool MainWindow::create() {
    if(!Engine::init()) {
        return false;
    }

    AbstractToolWindowManagerArea::registerClassFactory(&Engine::instance());
    ToolWindowManagerArea::registerClassFactory(&Engine::instance());
    ToolWindowManager::registerClassFactory(&Engine::instance());
    ImportQueue::registerClassFactory(&Engine::instance());
    Viewport::registerClassFactory(&Engine::instance());
    SceneComposer::registerClassFactory(&Engine::instance());
    HierarchyBrowser::registerClassFactory(&Engine::instance());
    ContentBrowser::registerClassFactory(&Engine::instance());
    SplinePanel::registerClassFactory(&Engine::instance());

    m_context = Engine::objectCreate<PipelineContext>("PipelineContext");

    if(!Window::create(1280, 800, Engine::applicationName().data(), Window::Windowed | Window::VSync | Window::Resizable)) {
        return false;
    }

    return true;
}

void MainWindow::onOpenProject(const TString &path) {
    //ProjectModel::addProject(path);

    Editor::project()->init(path);
    // Strict after project initialization
    Editor::assets()->init();
    // Read settings early for converters
    Editor::settings()->loadSettings();
    // Rescan project directory
    Editor::assets()->rescan();

    if(!Editor::plugins()->rescanProject(Editor::project()->pluginsPath())) {
        Editor::project()->currentBuilder(Editor::project()->currentPlatformName())->makeOutdated();
    }

    initUI();

    SceneComposer *composer = Widget::createWidget<SceneComposer>("SceneComposer", m_toolWindowManager);
    if(composer && Engine::world()->scenes().empty()) {
        composer->onNewAsset();
    }
    m_toolWindowManager->addToolWindow(composer, ToolWindowManager::EmptySpaceArea);
    m_hierarchyBrowser = Widget::createWidget<HierarchyBrowser>("HierarchyBrowser", m_toolWindowManager);
    if(m_hierarchyBrowser && composer) {
        AbstractToolWindowManagerArea *composerArea = m_toolWindowManager->areaFor(composer);
        if(composerArea) {
            m_toolWindowManager->addToolWindow(m_hierarchyBrowser, ToolWindowManager::ReferenceLeftOf, composerArea);
            connect(m_toolWindowManager, _SIGNAL(currentToolWindowChanged(Widget*)), this, _SLOT(onCurrentToolWindowChanged(Widget*)));
            onCurrentToolWindowChanged(composer);
        } else {
            aError() << "Unable to add HierarchyBrowser beside SceneComposer.";
        }
    } else {
        aError() << "Unable to create the HierarchyBrowser.";
    }
    ContentBrowser *contentBrowser = Widget::createWidget<ContentBrowser>("ContentBrowser", m_toolWindowManager);
    AbstractToolWindowManagerArea *contentArea = m_toolWindowManager->areaFor(composer);
    if(contentBrowser && contentArea) {
        m_toolWindowManager->addToolWindow(contentBrowser, ToolWindowManager::ReferenceBottomOf, contentArea);
        connect(contentBrowser, _SIGNAL(openEditor(TString)), this, _SLOT(onContentBrowserOpen(TString)));
        m_fileDropTarget.setDropHandler([contentBrowser](const StringList &files) {
            contentBrowser->importDroppedFiles(files);
        });
        setFileDropTarget(&m_fileDropTarget);
    } else {
        m_fileDropTarget.setDropHandler(FileDropTarget::DropHandler());
        setFileDropTarget(nullptr);
        aError() << "Unable to create the ContentBrowser.";
    }
    createMenus(composer);

    for(const TString &it : Editor::project()->platforms()) {
        TString name(it);
        //name.replace(0, 1, name.at(0).toUpper());
        //QAction *action = ui->menuBuild_Project->addAction(tr("Build for %1").arg(name));
        //action->setProperty(gPlatforms, it.data());
        //connect(action, &QAction::triggered, this, &MainWindow::onBuildProject);
    }

    //m_contentBrowser->createContextMenus();

    Editor::plugins()->initSystems();
}

void MainWindow::onImportFinished() {
    //SceneComposer *composer = new SceneComposer(this);
    //Editor::addEditor(composer);

    //m_preview = new Preview(this);

    Editor::settings()->loadSettings();

    //m_editorSettingsBrowser->init();
    //m_projectSettingsBrowser->init();

    //PropertyEditor *property = new PropertyEditor(this);
    //connect(m_contentBrowser, &ContentBrowser::assetsSelected, property, &PropertyEditor::onObjectSelected);
    //Editor::addGadget(property);
    //Editor::addGadget(new HierarchyBrowser(this));

    //for(auto it : Editor::gadgets()) {
    //    ui->toolWidget->addToolWindow(it, ToolWindowManager::NoArea);
    //}

    //ui->toolWidget->addToolWindow(m_preview, ToolWindowManager::NoArea);
    //ui->toolWidget->addToolWindow(composer, ToolWindowManager::NoArea);

    //foreach(QWidget *it, ui->toolWidget->toolWindows()) {
    //    QAction *action = new QAction(it->windowTitle(), ui->menuWindow);
    //    ui->menuWindow->addAction(action);
    //    action->setObjectName(it->windowTitle());
    //    action->setData(QVariant::fromValue(it));
    //    action->setCheckable(true);
    //    action->setChecked(false);

    //    connect(action, SIGNAL(triggered(bool)), this, SLOT(onToolWindowActionToggled(bool)));
    //}

    //QSettings settings(COMPANY_NAME, EDITOR_NAME);
    //// Load last session state
    //m_currentWorkspace = settings.value(gWorkspace, m_currentWorkspace).toString();
    //ui->toolPanel->setVisible(true);
    //ui->toolWidget->setVisible(true);
    //QVariant windows = settings.value(gWindows);
    //if(!windows.isValid() || !ui->toolWidget->restoreState(windows)) {
    //    on_actionReset_Workspace_triggered();
    //} else {
    //    for(auto it : ui->menuWorkspace->children()) {
    //        QAction *action = static_cast<QAction*>(it);
    //        action->blockSignals(true);
    //        action->setChecked((action->data().toString() == m_currentWorkspace));
    //        action->blockSignals(false);
    //    }
    //}
    //// Open the same editors with documents from the last session
    //QVariant map = settings.value(Editor::project()->projectId().data());
    //if(map.isValid()) {
    //    VariantList editors = Json::load(map.toString().toStdString()).toList();
    //    if(!editors.empty()) {
    //        for(auto &it : editors) {
    //            VariantList params = it.toList();
    //            if(params.size() == 2) {
    //                LegacyAssetEditor *editor = nullptr;
    //                // Documents
    //                for(auto &document : params.front().toList()) {
    //                    LegacyAssetEditor *e = openEditor(document.toList().front().toString());
    //                    if(e) {
    //                        editor = e;
    //                    }
    //                }

    //                if(editor) {
    //                    editor->restoreState(params.back().toMap());
    //                }
    //            }
    //        }
    //    }
    //}

    //if(composer->openedDocuments().empty()) {
    //    AssetManager *mgr = Editor::assets();
    //    TString firstMap = mgr->uuidToPath(Editor::project()->firstMap());

    //    AssetConverterSettings *mapSettings = mgr->fetchSettings(firstMap);
    //    if(mapSettings) {
    //        openEditor(firstMap);
    //    } else {
    //        static_cast<LegacyAssetEditor *>(composer)->onNewAsset();
    //    }
    //}

    // Set ui state
    disconnect(m_queue, nullptr, this, nullptr);

    //ComponentModel::instance()->update();

    //ui->actionNew->setEnabled(true);
    //ui->actionOpen->setEnabled(true);
    //ui->actionSave->setEnabled(true);
    //ui->actionSave_As->setEnabled(true);
    //ui->menuEdit->setEnabled(true);
    //ui->menuWindow->setEnabled(true);
    //ui->menuBuild_Project->setEnabled(true);

    Editor::plugins()->syncWhiteList();

    TString version(SDK_VERSION);
    if(Editor::project()->projectSdk() != version) {
        Editor::project()->setProjectSdk(version);
        Editor::project()->saveSettings();
    }

    //setGameMode(false);
}

void MainWindow::initUI() {
    m_canvas = Engine::objectCreate<Canvas>("Canvas", m_scene);
    if(!m_canvas) {
        aError() << "Unable to create the main UI canvas.";
        return;
    }

    Canvas *canvas = m_canvas;
    m_menuBarWidget = Widget::createWidget<MenuBar>("MenuBar", canvas);
    if(m_menuBarWidget) {
        if(!canvas || !m_menuBarWidget->attachToCanvas(canvas, this)) {
            aError() << "Unable to attach the UiKit menu bar to the main canvas.";
        }
    }

    m_toolWindowManager = Widget::createWidget<ToolWindowManager>("ToolWindowManager", canvas);
    m_toolWindowManager->setAnchors(Vector2(0.0f), Vector2(1.0f));
    m_toolWindowManager->setMargin(Vector4(m_menuBarWidget && !m_menuBarWidget->isUsingNativeMenuBar() ? kMenuBarHeight : 0.0f, 0.0f, kStatusBarHeight, 0.0f));

    Frame *statusBar = Widget::createWidget<Frame>("StatusBar", canvas);
    statusBar->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f));
    statusBar->setPivot(Vector2(0.0f, 0.0f));
    statusBar->setSize(Vector2(0.0f, kStatusBarHeight));
    statusBar->setBackgroundColor(Vector4(0.12f, 0.12f, 0.12f, 1.0f));

    m_queue = Widget::createWidget<ImportQueue>("ImportQueue", statusBar);
    m_queue->setAnchors(Vector2(0.0f), Vector2(1.0f));
    m_queue->setMargin(Vector4(4.0f));
    m_queue->setProgressColor(Vector4(0.25f, 0.65f, 0.95f, 1.0f));
    m_queue->setEnabled(false);
    connect(m_queue, _SIGNAL(importFinished()), this, _SLOT(onImportFinished()));

    RenderSystem *render = Engine::renderSystem();
    if(render) {
        if(m_context) {
            PipelineTask *task = dynamic_cast<PipelineTask *>(Engine::objectCreate("GuiLayer"));
            if(task) {
                m_canvas->skipRenderResult(true);
                task->setInput(0, m_canvas->texture());
                m_context->insertRenderTask(task);
            }
        }

        render->init();
    }
}

void MainWindow::createMenus(SceneComposer *composer) {
    if(!composer) {
        aError() << "Unable to create the main menu without a SceneComposer.";
        return;
    }

    if(!m_menuBarWidget) {
        aError() << "Unable to create main menus without a UiKit MenuBar.";
        return;
    }

    MenuBar::MenuModel *fileMenu = m_menuBarWidget->addMenu("File");
    if(!fileMenu) {
        aError() << "Unable to create the File menu.";
        return;
    }
    fileMenu->addAction("New Scene", [composer]() { composer->onNewAsset(); });
    fileMenu->addAction("Open Scene", [composer]() { composer->onOpenAsset(); });
    fileMenu->addAction("Save", [composer]() { composer->onSave(); });
    fileMenu->addAction("Save As", [composer]() { composer->onSaveAs(); });

    MenuBar::MenuModel *editMenu = m_menuBarWidget->addMenu("Edit");
    if(!editMenu) {
        aError() << "Unable to create the Edit menu.";
        return;
    }
    editMenu->addAction("Undo", [composer]() { composer->onUndoAction(); });
    editMenu->addAction("Redo", [composer]() { composer->onRedoAction(); });
    editMenu->addSeparator();
    editMenu->addAction("Cut", [composer]() { composer->onCutAction(); });
    editMenu->addAction("Copy", [composer]() { composer->onCopyAction(); });
    editMenu->addAction("Paste", [composer]() { composer->onPasteAction(); });
    editMenu->addAction("Delete", [composer]() { composer->onSelectionDeleted(); });

    m_windowMenu = m_menuBarWidget->addMenu("Window");
    if(!m_windowMenu) {
        aError() << "Unable to create the Window menu.";
        return;
    }
    m_windowMenuItems.clear();
    connect(m_toolWindowManager, _SIGNAL(toolWindowVisibilityChanged(Widget*)),
            this, _SLOT(onToolWindowVisibilityChanged(Widget*)));
    for(Widget *toolWindow : m_toolWindowManager->toolWindows()) {
        if(!toolWindow) {
            continue;
        }
        const bool visible = m_toolWindowManager->areaFor(toolWindow) != nullptr;
        const size_t index = m_windowMenu->addCheckableAction(toolWindow->name(), [this, toolWindow]() {
            onWindowMenuAction(toolWindow);
        }, visible);
        m_windowMenuItems.emplace_back(toolWindow, index);
    }
}

void MainWindow::onWindowMenuAction(Widget *toolWindow) {
    if(!m_toolWindowManager || !toolWindow) {
        return;
    }
    if(m_toolWindowManager->areaFor(toolWindow)) {
        m_toolWindowManager->hideToolWindow(toolWindow);
    } else {
        m_toolWindowManager->moveToolWindow(toolWindow, ToolWindowManager::LastUsedArea);
    }
}

void MainWindow::onToolWindowVisibilityChanged(Widget *toolWindow) {
    if(!m_toolWindowManager) {
        return;
    }
    const bool visible = m_toolWindowManager->areaFor(toolWindow) != nullptr;
    if(m_windowMenu) {
        for(const auto &item : m_windowMenuItems) {
            if(item.first == toolWindow) {
                m_windowMenu->setActionChecked(item.second, visible);
                break;
            }
        }
    }
    if(!visible && toolWindow == m_currentAssetEditor) {
        m_currentAssetEditor = nullptr;
        if(m_hierarchyBrowser) {
            m_hierarchyBrowser->setEditor(nullptr);
        }
    }
}

void MainWindow::onCurrentToolWindowChanged(Widget *toolWindow) {
    if(AssetEditor *editor = dynamic_cast<AssetEditor *>(toolWindow)) {
        m_currentAssetEditor = editor;
        if(m_hierarchyBrowser) {
            m_hierarchyBrowser->setEditor(editor);
        }
    }
}

void MainWindow::onContentBrowserOpen(const TString &path) {
    AssetEditor *target = m_currentAssetEditor;
    if(!target && m_toolWindowManager) {
        for(Widget *toolWindow : m_toolWindowManager->toolWindows()) {
            target = dynamic_cast<AssetEditor *>(toolWindow);
            if(target) {
                break;
            }
        }
    }
    if(!target) {
        aWarning() << "No active asset editor is available to open:" << path;
        return;
    }
    AssetConverterSettings *settings = Editor::assets()->fetchSettings(path);
    if(!settings) {
        aWarning() << "Unable to load asset settings for:" << path;
        return;
    }
    target->loadAsset(settings);
    if(m_toolWindowManager && !m_toolWindowManager->areaFor(target)) {
        m_toolWindowManager->moveToolWindow(target, ToolWindowManager::LastUsedArea);
    }
    if(m_toolWindowManager) {
        m_toolWindowManager->activateToolWindow(target);
    }
}

void MainWindow::beforeRender() {
    if(!Engine::isGameMode()/* || (m_preview && m_preview->isPaused())*/) {
        Timer::update();
    }

    if(m_queue) {
        m_queue->pollImport();
    }
}
