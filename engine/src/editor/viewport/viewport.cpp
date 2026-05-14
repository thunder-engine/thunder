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
#include <QMouseEvent>

#include <systems/rendersystem.h>
#include <systems/resourcesystem.h>

#include <components/actor.h>
#include <components/camera.h>
#include <components/world.h>

#include <resources/pipeline.h>

#include <pipelinecontext.h>
#include <timer.h>

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_controller(nullptr),
        m_world(nullptr),
        m_camera(nullptr),
        m_guiLayer(nullptr),
        m_outlinePass(nullptr),
        m_gizmoRender(nullptr),
        m_gridRender(nullptr),
        m_debugRender(nullptr),
        m_pipelineContext(nullptr),
        m_rhiWindow(nullptr),
        m_color(nullptr),
        m_focusedView(false),
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
}

void Viewport::init() {
    m_pipelineContext = Engine::objectCreate<PipelineContext>("PipelineContext");
    m_pipelineContext->setPipeline(Engine::loadResource<Pipeline>(Engine::value(".pipeline", ".embedded/Deferred.pipeline").toString()));
    m_rhiWindow = Engine::renderSystem()->createRhiWindow(this);

    if(m_rhiWindow) {
        static bool first = true;
        if(first) {
            QWindow *testRhi = Engine::renderSystem()->createRhiWindow(this);
            testRhi->show();

            Engine::renderSystem()->init();

            delete testRhi;
            first = false;
        }

        m_rhiWindow->installEventFilter(this);
        layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));

        if(!m_pipelineContext->renderTasks().empty()) {
            m_guiLayer = m_pipelineContext->renderTasks().back();

            m_color = m_pipelineContext->resultTexture();
            m_color->setFlags(m_color->flags() | Texture::Feedback);

            m_debugRender = new DebugRender;

            if(!m_gameView) {
                m_gridRender = new GridRender;
                m_gridRender->setController(m_controller);
                m_gridRender->setInput(0, m_guiLayer->output(0));
                m_gridRender->setInput(1, m_pipelineContext->textureBuffer("depthMap"));

                m_outlinePass = new Outline;
                m_outlinePass->setController(m_controller);
                m_outlinePass->setInput(0, m_guiLayer->output(0));

                m_gizmoRender = new GizmoRender;
                m_gizmoRender->setController(m_controller);
                m_gizmoRender->setInput(0, m_guiLayer->output(0));
                m_gizmoRender->setInput(1, m_pipelineContext->textureBuffer("depthMap"));

                if(m_controller) {
                    m_controller->init(this);
                }

                m_pipelineContext->insertRenderTask(m_gridRender, m_guiLayer);
                m_pipelineContext->insertRenderTask(m_outlinePass, m_guiLayer);
                m_pipelineContext->insertRenderTask(m_gizmoRender);

                Handles::init();
            }

            m_pipelineContext->insertRenderTask(m_debugRender, m_guiLayer);
            m_pipelineContext->subscribePost(Viewport::readPixels, this);
        }
    }
}

CameraController *Viewport::controller() {
    return m_controller;
}

void Viewport::setController(CameraController *ctrl) {
    m_controller = ctrl;
}

void Viewport::setWorld(World *world) {
    if(m_world != world) {
        m_world = world;

        if(m_controller) {
            m_controller->setActiveRootObject(m_world);
        }
    }
}

void Viewport::setCamera(Camera *camera) {
    m_camera = camera;
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

void Viewport::onDraw() {
    if(m_frameInProgress) {
        return;
    }

    m_frameInProgress = true; // Recursive call protection

    if(m_world) {
        m_world->setActive(m_liveUpdate);

        auto &instance = EditorPlatform::instance();
        instance.setScreenSize(size());

        m_pipelineContext->resize(width(), height());
        Engine::renderSystem()->setPipelineContext(m_pipelineContext);

        if(m_gameView) {
            for(auto it : m_world->findChildren<Camera *>()) {
                if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                    Camera::setCurrent(it);
                    break;
                }
            }

            Engine::update(m_world);

            if(!m_gamePaused && isFocused()) {
                QPoint p = mapFromGlobal(QCursor::pos());
                instance.setMousePosition(p);
                instance.setMouseDelta(p - m_savedMousePos);

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
            if(m_controller) {
                Camera::setCurrent(m_controller->camera());

                m_controller->resize(width(), height());
                m_controller->move();
            }

            Engine::instance().processEvents();
            Engine::renderSystem()->setActiveWorld(m_world);
            Engine::renderSystem()->processEvents();
        }

        if(isFocused()) {
            if(!m_gameView && m_controller) {
                m_controller->update();
            }
            onCursorSet(instance.mouseCursor());
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
    QMenu *tasksMenu = menu->addMenu(tr("Render Tasks"));
    for(auto &it : m_pipelineContext->renderTasks()) {
        if(!it->name().isEmpty()) {
            QAction *action = addAction(tasksMenu, it->name());
            action->setChecked(it->isEnabled());

            QObject::connect(action, &QAction::triggered, this, &Viewport::onPostEffectChanged);
        }
    }

    if(m_debugRender) {
        QMenu *bufferMenu = menu->addMenu(tr("Buffer Visualization"));
        for(auto &it : m_pipelineContext->renderTextures()) {
            if(!it.isEmpty()) {
                QAction *action = addAction(bufferMenu, it);
                action->setChecked(m_debugRender->isBufferVisible(it.data()));

                QObject::connect(action, &QAction::triggered, this, &Viewport::onBufferChanged);
            }
        }
    }
}

PipelineContext *Viewport::pipelineContext() const {
    return m_pipelineContext;
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
    if(m_gridRender) {
        m_gridRender->setEnabled(enabled);
    }
}
void Viewport::setGizmoEnabled(bool enabled) {
    if(m_gizmoRender) {
        m_gizmoRender->setEnabled(enabled);
    }
}
void Viewport::setOutlineEnabled(bool enabled) {
    if(m_outlinePass) {
        m_outlinePass->setEnabled(enabled);
    }
}
void Viewport::setGuiEnabled(bool enabled) {
    if(m_guiLayer) {
        m_guiLayer->setEnabled(enabled);
    }
}

void Viewport::showCube(bool enabled) {
    if(m_gizmoRender) {
        m_gizmoRender->showCube(enabled);
    }
}

void Viewport::showGizmos(bool enabled) {
    if(m_gizmoRender) {
        m_gizmoRender->showGizmos(enabled);
    }
}

void Viewport::onInProgressFlag(bool flag) {
    m_frameInProgress = flag;
}

void Viewport::addRenderTask(PipelineTask *task) {
    m_pipelineContext->insertRenderTask(task, m_pipelineContext->renderTasks().empty() ? nullptr : m_pipelineContext->renderTasks().front());
}

void Viewport::readPixels(void *object) {
    Viewport *ptr = reinterpret_cast<Viewport *>(object);
    if(ptr && ptr->m_screenInProgress && ptr->m_color) {
        ptr->m_color->readPixels(0, 0, ptr->m_color->width(), ptr->m_color->height());
    }
}

bool Viewport::isFocused() {
    bool focus = false;
#ifdef Q_OS_MACOS
    focus = rect().contains(mapFromGlobal(QCursor::pos()));
#else
    focus = QGuiApplication::focusWindow() == m_rhiWindow;
#endif
    if(focus != m_focusedView) {
        m_focusedView = focus;
        EditorPlatform::instance().reset();
    }

    return focus;
}

QAction *Viewport::addAction(QMenu *menu, const TString &name) {
    static const QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
    static const QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

    QString result(name.data());
    result.replace(regExp1, "\\1 \\2");
    result.replace(regExp2, "\\1 \\2");
    result.replace(0, 1, result[0].toUpper());

    QAction *action = menu->addAction(result);
    action->setCheckable(true);
    action->setData(name.data());

    return action;
}

void Viewport::onBufferChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        m_debugRender->trackBuffer(action->data().toString().toStdString(), checked);
    }
}

void Viewport::onPostEffectChanged(bool checked) {
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if(action) {
        for(auto &it : m_pipelineContext->renderTasks()) {
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
    bool filter = isFocused();
    if(event->type() == QEvent::DragEnter) {
        m_rhiWindow->requestActivate();
        filter = true;
    }

    if(filter && processEvent(event)) {
        return true;
    }

    return QObject::eventFilter(object, event);
}

bool Viewport::processEvent(QEvent *event) {
    switch(event->type()) {
        case QEvent::DragEnter: {
            QDragEnterEvent *ev = static_cast<QDragEnterEvent *>(event);
            EditorPlatform::instance().setMousePosition(ev->pos());

            emit dragEnter(ev);
            return true;
        }
        case QEvent::DragLeave: emit dragLeave(static_cast<QDragLeaveEvent *>(event)); return true;
        case QEvent::DragMove: {
            QDragMoveEvent *ev = static_cast<QDragMoveEvent *>(event);
            EditorPlatform::instance().setMousePosition(ev->pos());

            emit dragMove(ev);
            return true;
        }
        case QEvent::Drop: {
            QDropEvent *ev = static_cast<QDropEvent *>(event);
            EditorPlatform::instance().setMousePosition(ev->pos());

            emit drop(ev);
            return true;
        }
        case QEvent::KeyPress: {
            EditorPlatform::instance().setKeys(static_cast<QKeyEvent *>(event), false);
            return true;
        }
        case QEvent::KeyRelease: {
            EditorPlatform::instance().setKeys(static_cast<QKeyEvent *>(event), true);
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
            EditorPlatform::instance().setMousePosition(ev->pos());
            return true;
        }
        default: break;
    }

    return false;
}
