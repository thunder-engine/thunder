#include "editor/viewport/viewport.h"
#include "editor/viewport/cameracontroller.h"
#include "editor/viewport/handles.h"

#include "editor/pluginmanager.h"

#include "editor/editorplatform.h"

#include "editor/viewport/tasks/debugrender.h"
#include "editor/viewport/tasks/gizmorender.h"
#include "editor/viewport/tasks/gridrender.h"
#include "editor/viewport/tasks/outlinerender.h"

#include <QWindow>
#include <QMenu>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QRegularExpression>

#include <systems/rendersystem.h>
#include <systems/resourcesystem.h>

#include <components/actor.h>
#include <components/camera.h>
#include <components/world.h>

#include <pipelinecontext.h>

#include <editorsettings.h>

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_controller(nullptr),
        m_world(nullptr),
        m_guiLayer(nullptr),
        m_outlinePass(nullptr),
        m_gizmoRender(nullptr),
        m_gridRender(nullptr),
        m_debugRender(nullptr),
        m_renderSystem(nullptr),
        m_rhiWindow(nullptr),
        m_tasksMenu(nullptr),
        m_bufferMenu(nullptr),
        m_color(nullptr),
        m_gameView(false),
        m_gamePaused(false),
        m_liveUpdate(false),
        m_frameInProgress(false),
        m_screenInProgress(false) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);
#ifdef Q_OS_MACOS
    setFocusPolicy(Qt::StrongFocus);
#endif

    QObject::connect(EditorSettings::instance(), &EditorSettings::updated, this, &Viewport::onApplySettings);
}

void Viewport::init() {
    m_renderSystem = (m_gameView) ? Engine::renderSystem() : PluginManager::instance()->createRenderer();

    m_rhiWindow = m_renderSystem->createRhiWindow(this);

    if(m_rhiWindow) {
        static bool first = true;
        if(first) {
            QWindow *testRhi = m_renderSystem->createRhiWindow(this);
            testRhi->show();

            m_renderSystem->init();

            delete testRhi;
            first = false;
        } else {
            m_renderSystem->init();
        }

        m_rhiWindow->installEventFilter(this);
        layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));

        PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
        m_guiLayer = pipelineContext->renderTasks().back();

        m_color = pipelineContext->resultTexture();
        m_color->setFlags(m_color->flags() | Texture::Feedback);

        m_debugRender = new DebugRender;

        if(!m_gameView) {
            m_gridRender = new GridRender;
            m_gridRender->setController(m_controller);
            m_gridRender->setInput(0, m_guiLayer->output(0));
            m_gridRender->setInput(1, pipelineContext->textureBuffer("depthMap"));

            m_outlinePass = new Outline;
            m_outlinePass->setController(m_controller);
            m_outlinePass->setInput(0, m_guiLayer->output(0));

            m_gizmoRender = new GizmoRender;
            m_gizmoRender->setController(m_controller);
            m_gizmoRender->setInput(0, m_guiLayer->output(0));
            m_gizmoRender->setInput(1, pipelineContext->textureBuffer("depthMap"));

            if(m_controller) {
                m_controller->init(this);
            }

            pipelineContext->insertRenderTask(m_gridRender, m_guiLayer);
            pipelineContext->insertRenderTask(m_outlinePass, m_guiLayer);
            pipelineContext->insertRenderTask(m_gizmoRender);

            Handles::init();
        }

        pipelineContext->insertRenderTask(m_debugRender, m_guiLayer);
        pipelineContext->subscribePost(Viewport::readPixels, this);
    }
}

CameraController *Viewport::controller() {
    return m_controller;
}

void Viewport::setController(CameraController *ctrl) {
    m_controller = ctrl;

    connect(m_controller, &CameraController::setCursor, this, &Viewport::onCursorSet);
    connect(m_controller, &CameraController::unsetCursor, this, &Viewport::onCursorUnset);
}

void Viewport::setWorld(World *world) {
    if(m_world != world) {
        m_world = world;

        if(m_controller) {
            m_controller->setActiveRootObject(m_world);
        }
    }
}

void Viewport::onCursorSet(const QCursor &cursor) {
    if(m_rhiWindow) {
        m_rhiWindow->setCursor(cursor);
    }
}

void Viewport::onCursorUnset() {
    if(m_rhiWindow) {
        m_rhiWindow->unsetCursor();
    }
}

void Viewport::onApplySettings() {
    if(m_outlinePass) {
        m_outlinePass->loadSettings();
    }
    if(m_gridRender) {
        m_gridRender->loadSettings();
    }
}

void Viewport::onDraw() {
    if(m_frameInProgress) {
        return;
    }

    m_frameInProgress = true; // Recursive call protection

    if(m_world) {
        if(m_liveUpdate) {
            m_world->setToBeUpdated(true);
        }

        auto &instance = EditorPlatform::instance();

        instance.setScreenSize(size());

        if(m_gameView) {
            for(auto it : m_world->findChildren<Camera *>()) {
                if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                    Camera::setCurrent(it);
                    break;
                }
            }

            if(!m_gamePaused && isFocused()) {
                QPoint p = mapFromGlobal(QCursor::pos());
                instance.setMousePosition(p);
                instance.setMouseDelta(p - m_savedMousePos);

                Engine::update();

                if(instance.isMouseLocked() && Engine::isGameMode()) {
                    m_savedMousePos = QPoint(width() / 2, height() / 2);
                    QCursor::setPos(mapToGlobal(m_savedMousePos));

                    m_rhiWindow->setCursor(Qt::BlankCursor);
                } else {
                    m_savedMousePos = p;

                    m_rhiWindow->setCursor(Qt::ArrowCursor);
                }
            }
        } else {
            Engine::resourceSystem()->processEvents();

            if(m_controller) {
                Camera::setCurrent(m_controller->camera());

                m_controller->resize(width(), height());
                m_controller->move();
            }
        }

        m_renderSystem->pipelineContext()->resize(width(), height());
        m_renderSystem->update(m_world);

        if(isFocused()) {
            if(!m_gameView) {
                m_controller->update();
            }
            instance.update();
        }

        if(m_screenInProgress && m_color) {
            ByteArray data(m_color->getPixels(0));

            emit screenshot(QImage(data.data(), m_color->width(), m_color->height(), QImage::Format_RGBA8888).mirrored());

            m_screenInProgress = false;
        }
    }

    m_frameInProgress = false;
}

void Viewport::createMenu(QMenu *menu) {
    m_tasksMenu = menu->addMenu(tr("Render Tasks"));
    m_bufferMenu = menu->addMenu(tr("Buffer Visualization"));

    fillTasksMenu(m_tasksMenu);

    QObject::connect(m_bufferMenu, &QMenu::aboutToShow, this, &Viewport::onBufferMenu);
}

PipelineContext *Viewport::pipelineContext() const {
    return m_renderSystem->pipelineContext();
}

void Viewport::grabScreen() {
    m_screenInProgress = true;
}

int Viewport::gridCell() {
    return m_gridRender->scale();
}

bool Viewport::isGamePaused() const {
    return m_gamePaused;
}

void Viewport::setGamePaused(bool pause) {
    m_gamePaused = pause;
    if(m_gamePaused) {
        m_rhiWindow->setCursor(Qt::ArrowCursor);
    }
}

bool Viewport::isLiveUpdate() const {
    return m_liveUpdate;
}

void Viewport::setLiveUpdate(bool update) {
    m_liveUpdate = update;
}

void Viewport::setGameView(bool enabled) {
    m_gameView = enabled;
}

void Viewport::setGridEnabled(bool enabled) {
    m_gridRender->setEnabled(enabled);
}
void Viewport::setGizmoEnabled(bool enabled) {
    m_gizmoRender->setEnabled(enabled);
}
void Viewport::setOutlineEnabled(bool enabled) {
    m_outlinePass->setEnabled(enabled);
}
void Viewport::setGuiEnabled(bool enabled) {
    m_guiLayer->setEnabled(enabled);
}

void Viewport::showCube(bool enabled) {
    m_gizmoRender->showCube(enabled);
}

void Viewport::showGizmos(bool enabled) {
    m_gizmoRender->showGizmos(enabled);
}

void Viewport::onInProgressFlag(bool flag) {
    m_frameInProgress = flag;
}

void Viewport::addRenderTask(PipelineTask *task) {
    PipelineContext *pipelineContext = m_renderSystem->pipelineContext();
    pipelineContext->insertRenderTask(task, pipelineContext->renderTasks().front());
}

void Viewport::readPixels(void *object) {
    Viewport *ptr = reinterpret_cast<Viewport *>(object);
    if(ptr && ptr->m_screenInProgress && ptr->m_color) {
        ptr->m_color->readPixels(0, 0, ptr->m_color->width(), ptr->m_color->height());
    }
}

bool Viewport::isFocused() const {
#ifdef Q_OS_MACOS
    return rect().contains(mapFromGlobal(QCursor::pos()));
#else
    return (QGuiApplication::focusWindow() == m_rhiWindow);
#endif
}

void Viewport::onBufferMenu() {
    if(m_bufferMenu && m_debugRender) {
        m_bufferMenu->clear();

        StringList list = m_renderSystem->pipelineContext()->renderTextures();

        for(auto &it : list) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result(it.data());
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = m_bufferMenu->addAction(result);
            action->setData(it.data());
            action->setCheckable(true);
            action->setChecked(m_debugRender->isBufferVisible(it.data()));

            QObject::connect(action, &QAction::triggered, this, &Viewport::onBufferChanged);
        }
    }
}

void Viewport::fillTasksMenu(QMenu *menu) {
    if(menu) {
        menu->clear();

        PipelineContext *context = m_renderSystem->pipelineContext();
        for(auto &it : context->renderTasks()) {
            static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
            static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

            QString result(it->name().data());
            if(result.isEmpty()) {
                continue;
            }
            result.replace(regExp1, "\\1 \\2");
            result.replace(regExp2, "\\1 \\2");
            result.replace(0, 1, result[0].toUpper());

            QAction *action = menu->addAction(result);
            action->setCheckable(true);
            action->setChecked(it->isEnabled());
            action->setData(it->name().data());

            QObject::connect(action, &QAction::toggled, this, &Viewport::onPostEffectChanged);
        }
    }
}

void Viewport::onBufferChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        std::string buffer = action->data().toString().toStdString();
        if(checked) {
            m_debugRender->showBuffer(buffer);
        } else {
            m_debugRender->hideBuffer(buffer);
        }
    }
}

void Viewport::onPostEffectChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        for(auto &it : m_renderSystem->pipelineContext()->renderTasks()) {
            if(action->data().toString().toStdString() == it->name().toStdString()) {
                it->setEnabled(checked);
            }
        }
    }
}

bool Viewport::event(QEvent *event) {
    if(processEvent(event)) {
        return true;
    }

    return QWidget::event(event);
}

bool Viewport::eventFilter(QObject *object, QEvent *event) {
    if(isFocused() && processEvent(event)) {
        return true;
    }

    return QObject::eventFilter(object, event);
}

bool Viewport::processEvent(QEvent *event) {
    switch(event->type()) {
        case QEvent::DragEnter: emit dragEnter(static_cast<QDragEnterEvent *>(event)); return true;
        case QEvent::DragLeave: emit dragLeave(static_cast<QDragLeaveEvent *>(event)); return true;
        case QEvent::DragMove: emit dragMove(static_cast<QDragMoveEvent *>(event)); return true;
        case QEvent::Drop: emit drop(static_cast<QDropEvent *>(event)); setFocus(); return true;
        case QEvent::KeyPress: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), ev->text(), false, ev->isAutoRepeat());
            return true;
        }
        case QEvent::KeyRelease: {
            QKeyEvent *ev = static_cast<QKeyEvent *>(event);
            EditorPlatform::instance().setKeys(ev->key(), "", true, ev->isAutoRepeat());
            return true;
        }
        case QEvent::Wheel: {
            QWheelEvent *ev = static_cast<QWheelEvent *>(event);
            int delta = ev->angleDelta().y();
            if(delta != 0) {
                EditorPlatform::instance().setMouseScrollDelta(delta);
            }
            return true;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setScreenSize(size());
            EditorPlatform::instance().setMousePosition(ev->pos());
            EditorPlatform::instance().setMouseButtons(ev->button(), PRESS);
            return true;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setMousePosition(ev->pos());
            EditorPlatform::instance().setMouseButtons(ev->button(), RELEASE);
            return true;
        }
        case QEvent::MouseMove: {
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);
            EditorPlatform::instance().setScreenSize(size());
            EditorPlatform::instance().setMousePosition(ev->pos());
            return true;
        }
        default: break;
    }

    return false;
}

void Viewport::resizeEvent(QResizeEvent *event) {
    EditorPlatform::instance().setScreenSize(event->size());
}
