#include "editor/viewport/viewport.h"

#include "editor/viewport/cameractrl.h"

#include "editor/pluginmanager.h"

#include <QWindow>
#include <QVBoxLayout>
#include <QGuiApplication>

#include <systems/rendersystem.h>

#include <components/camera.h>
#include <pipelinecontext.h>

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_controller(nullptr),
        m_sceneGraph(nullptr),
        m_rhiWindow(nullptr) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    RenderSystem *render = PluginManager::instance()->render();
    if(render) {
        m_rhiWindow = PluginManager::instance()->render()->createRhiWindow();

        m_rhiWindow->installEventFilter(this);
        layout()->addWidget(QWidget::createWindowContainer(m_rhiWindow));
    }

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void Viewport::init() {
    if(m_rhiWindow) {
        connect(m_rhiWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);
    }
}

void Viewport::setController(CameraCtrl *ctrl) {
    m_controller = ctrl;
}

void Viewport::setSceneGraph(SceneGraph *sceneGraph) {
    m_sceneGraph = sceneGraph;
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
    if(m_controller) {
        m_controller->update();

        Camera *camera = m_controller->camera();
        if(camera) {
            PipelineContext *pipe = camera->pipeline();
            pipe->resize(width(), height());
        }
        Camera::setCurrent(camera);
    }
    if(m_sceneGraph) {
        Engine::resourceSystem()->processEvents();

        RenderSystem *render = PluginManager::instance()->render();
        if(render) {
            render->update(m_sceneGraph);
        }
        Camera::setCurrent(nullptr);
    }
}
bool Viewport::eventFilter(QObject *object, QEvent *event) {
    switch(event->type()) {
    case QEvent::DragEnter: emit dragEnter(static_cast<QDragEnterEvent *>(event)); return true;
    case QEvent::DragLeave: emit dragLeave(static_cast<QDragLeaveEvent *>(event)); return true;
    case QEvent::DragMove: emit dragMove(static_cast<QDragMoveEvent *>(event)); return true;
    case QEvent::Drop: emit drop(static_cast<QDropEvent *>(event)); setFocus(); return true;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Wheel:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove: {
        // Workaround for the modal dialogs on top of RHI window and events propagation on to RHI
        if(m_controller && m_rhiWindow == QGuiApplication::focusWindow()) {
            m_controller->onInputEvent(static_cast<QMouseEvent *>(event));
        }
        return true;
    }
    default: break;
    }

    return QObject::eventFilter(object, event);
}
