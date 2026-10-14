#include "mainwindow.h"
#include "viewport/viewport.h"
#include "viewport/cameracontroller.h"

#include <engine.h>
#include <pipelinecontext.h>
#include <pipelinetask.h>
#include <world.h>
#include <log.h>
#include <timer.h>

#include <canvas.h>
#include <frame.h>
#include <recttransform.h>
#include <transform.h>

#include <editor.h>
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

namespace {
const float kStatusBarHeight = 24.0f;
}

MainWindow::MainWindow() :
        m_scene(nullptr),
        m_canvas(nullptr),
        m_toolWindowManager(nullptr),
        m_queue(nullptr) {

    m_world = dynamic_cast<World *>(Engine::objectCreate("World", "MainWorld"));
    m_scene = dynamic_cast<Scene *>(Engine::objectCreate("Scene", "MainScene", m_world));
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

    Scene *scene = dynamic_cast<Scene *>(Engine::objectCreate("Scene", "MainScene", Engine::world()));
    if(!scene) {
        aError() << "Unable to create Studio test scene";
        return;
    }

    Actor *viewportActor = Engine::composeActor<Viewport>("Viewport", m_toolWindowManager->actor());
    Viewport *viewport = viewportActor ? viewportActor->getComponent<Viewport>() : nullptr;
    if(!viewport) {
        aError() << "Unable to create Studio test viewport";
        return;
    }
    viewport->showCube(true);
    viewport->setWorld(Engine::world());
    viewport->setController(new CameraController);
    m_toolWindowManager->addToolWindow(viewport, ToolWindowManager::EmptySpaceArea);

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
    m_canvas = Engine::composeActor<Canvas>("Canvas", m_scene);

    Actor *managerActor = Engine::composeActor<ToolWindowManager>("ToolWindowManager", m_canvas);
    m_toolWindowManager = managerActor->getComponent<ToolWindowManager>();
    RectTransform *toolWindowManagerRect = m_toolWindowManager->rectTransform();
    toolWindowManagerRect->setAnchors(Vector2(0.0f), Vector2(1.0f));
    toolWindowManagerRect->setMargin(Vector4(0.0f, 0.0f, kStatusBarHeight, 0.0f));

    Actor *statusBarActor = Engine::composeActor<Frame>("StatusBar", m_canvas);
    Frame *statusBar = statusBarActor->getComponent<Frame>();
    RectTransform *statusBarRect = statusBar->rectTransform();
    statusBarRect->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f));
    statusBarRect->setPivot(Vector2(0.0f, 0.0f));
    statusBarRect->setSize(Vector2(0.0f, kStatusBarHeight));
    statusBar->setBackgroundColor(Vector4(0.12f, 0.12f, 0.12f, 1.0f));

    Actor *progressActor = Engine::composeActor<ImportQueue>("ImportQueue", statusBarActor);
    m_queue = progressActor->getComponent<ImportQueue>();
    RectTransform *progressRect = m_queue->rectTransform();
    progressRect->setAnchors(Vector2(0.0f), Vector2(1.0f));
    progressRect->setMargin(Vector4(4.0f));
    m_queue->setProgressColor(Vector4(0.25f, 0.65f, 0.95f, 1.0f));
    m_queue->actor()->setEnabled(false);
    connect(m_queue, _SIGNAL(importFinished()), this, _SLOT(onImportFinished()));

    RenderSystem *render = Engine::renderSystem();
    if(render) {
        if(m_context) {
            PipelineTask *task = dynamic_cast<PipelineTask *>(Engine::objectCreate("GuiLayer"));
            if(task) {
                Canvas *canvas = m_canvas->getComponent<Canvas>();
                if(canvas) {
                    canvas->skipRenderResult(true);
                    task->setInput(0, canvas->texture());
                }
                m_context->insertRenderTask(task);
            }
        }
        render->init();
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
