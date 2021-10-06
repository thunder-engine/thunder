#include "viewport.h"

#include <QWindow>
#include <QVBoxLayout>

#include <systems/rendersystem.h>

#include <components/camera.h>
#include <components/scene.h>

#include <resources/pipeline.h>

#include "controllers/cameractrl.h"

#include "pluginmanager.h"

Viewport::Viewport(QWidget *parent) :
        QWidget(parent),
        m_pController(nullptr),
        m_pScene(nullptr),
        m_pRHIWindow(nullptr) {

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    m_pRHIWindow = PluginManager::instance()->render()->createRhiWindow();
    m_pRHIWindow->installEventFilter(this);

    connect(m_pRHIWindow, SIGNAL(draw()), this, SLOT(onDraw()), Qt::DirectConnection);

    QWidget *widget = QWidget::createWindowContainer(m_pRHIWindow);

    layout()->addWidget(widget);

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
    PluginManager::instance()->addScene(m_pScene);
}

void Viewport::onDraw() {
    if(m_pController) {
        m_pController->update();
    }
    if(m_pScene) {
        findCamera();

        PluginManager::instance()->updateRender(m_pScene);
    }
}

void Viewport::findCamera() {
    if(m_pController) {
        Camera *camera = m_pController->camera();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width(), height());
            pipe->setTarget(0); //defaultFramebufferObject()
        }
        Camera::setCurrent(camera);
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
        if(m_pController) {
            m_pController->onInputEvent(static_cast<QInputEvent *>(event));
        }
        return true;
    }
    default: break;
    }
    return QObject::eventFilter(object, event);
}
