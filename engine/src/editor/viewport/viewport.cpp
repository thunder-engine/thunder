#include "editor/viewport/viewport.h"

#include "editor/viewport/cameractrl.h"

#include "editor/pluginmanager.h"

#include <QWindow>
#include <QVBoxLayout>
#include <QGuiApplication>

#include <systems/rendersystem.h>

#include <components/camera.h>
#include <resources/pipeline.h>

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_pController(nullptr),
        m_pScene(nullptr),
        m_pRHIWindow(nullptr) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    RenderSystem *render = PluginManager::instance()->render();
    if(render) {
        m_pRHIWindow = PluginManager::instance()->render()->createRhiWindow();
        if(m_pRHIWindow) {
            m_pRHIWindow->installEventFilter(this);

            connect(m_pRHIWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

            layout()->addWidget(QWidget::createWindowContainer(m_pRHIWindow));
        }
    }

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void Viewport::setController(CameraCtrl *ctrl) {
    m_pController = ctrl;
}

void Viewport::setScene(Scene *scene) {
    m_pScene = scene;
}

void Viewport::onCursorSet(const QCursor &cursor) {
    if(m_pRHIWindow) {
        m_pRHIWindow->setCursor(cursor);
    }
}

void Viewport::onCursorUnset() {
    if(m_pRHIWindow) {
        m_pRHIWindow->unsetCursor();
    }
}

void Viewport::onDraw() {
    if(m_pController) {
        m_pController->update();

        Camera *camera = m_pController->camera();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width(), height());
        }
        Camera::setCurrent(camera);
    }
    if(m_pScene) {
        Engine::resourceSystem()->processEvents();

        RenderSystem *render = PluginManager::instance()->render();
        if(render) {
            render->update(m_pScene);
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
        if(m_pController && m_pRHIWindow == QGuiApplication::focusWindow()) {
            m_pController->onInputEvent(static_cast<QInputEvent *>(event));
        }
        return true;
    }
    default: break;
    }

    return QObject::eventFilter(object, event);
}
