#include "viewport.h"

#include <system.h>
#include <commandbuffer.h>

#include <components/camera.h>

#include <resources/pipeline.h>

#include "handles.h"
#include "controllers/objectctrl.h"

#include "commandbuffer.h"

#include "pluginmodel.h"

#define OVERRIDE "uni.texture0"

Viewport::Viewport(QWidget *parent) :
        SceneView(parent),
        m_pCommandBuffer(nullptr)  {

    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    //setContextMenuPolicy(Qt::CustomContextMenu);
    setAutoFillBackground(false);
}

void Viewport::onSetMode() {
    if(m_Target.empty()) {
        m_Target    = "normalsMap";
    } else {
        m_Target.clear();
    }
}

void Viewport::initializeGL() {
    SceneView::initializeGL();
    PluginModel::instance()->initSystems();
    m_pCommandBuffer = Engine::objectCreate<ICommandBuffer>();
}

void Viewport::paintGL() {
    SceneView::paintGL();

    if(m_pController) {
        if(!m_Target.empty()) {
            Pipeline *pipeline  = Camera::current()->pipeline();

            MaterialInstance *sprite    = pipeline->sprite();
            sprite->setTexture(OVERRIDE, reinterpret_cast<const Texture *>(pipeline->target(m_Target)));

            m_pCommandBuffer->setScreenProjection();
            m_pCommandBuffer->drawMesh(Matrix4(), pipeline->plane(), 0, ICommandBuffer::UI, sprite);
        } else {
            m_pController->drawHandles(m_pCommandBuffer);
        }
    }
}

void Viewport::resizeGL(int width, int height) {
    SceneView::resizeGL(width, height);

    if(m_pController) {
        Camera::setCurrent(m_pController->camera());
        m_pController->resize(width, height);
    }
}

void Viewport::dragEnterEvent(QDragEnterEvent *event) {
    emit dragEnter(event);
}

void Viewport::dragMoveEvent(QDragMoveEvent *event) {
    emit dragMove(event);
}

void Viewport::dragLeaveEvent(QDragLeaveEvent *event) {
    emit dragLeave(event);
}

void Viewport::dropEvent(QDropEvent *event) {
    setFocus();
    emit drop(event);
}

void Viewport::mouseMoveEvent(QMouseEvent *pe) {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::mousePressEvent(QMouseEvent *pe) {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::mouseReleaseEvent(QMouseEvent *pe) {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::wheelEvent(QWheelEvent *pe) {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::keyPressEvent(QKeyEvent *pe) {
    QWidget::keyPressEvent(pe);
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::keyReleaseEvent(QKeyEvent *pe) {
    QWidget::keyReleaseEvent(pe);
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::findCamera() {
    if(m_pController) {
        Camera *camera = m_pController->camera();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width(), height());
            pipe->setTarget(defaultFramebufferObject());
        }
        Camera::setCurrent(camera);
    }
}
