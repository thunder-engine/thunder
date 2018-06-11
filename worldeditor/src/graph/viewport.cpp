#include "viewport.h"

#include <system.h>
#include <commandbuffer.h>

#include "handles.h"
#include "controllers/objectctrl.h"

#include "commandbuffer.h"

Viewport::Viewport(QWidget *parent) :
        SceneView(parent),
        m_pCommandBuffer(nullptr)  {

    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    //setContextMenuPolicy(Qt::CustomContextMenu);
    setAutoFillBackground(false);

}

void Viewport::initializeGL() {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->init(m_pScene);
    }
    SceneView::initializeGL();

    makeCurrent();

    m_pCommandBuffer    = Engine::objectCreate<ICommandBuffer>();

    Handles::init();
}

void Viewport::paintGL() {
    SceneView::paintGL();

    if(m_pController) {
        Handles::s_ActiveCamera = m_pController->activeCamera();
        Handles::beginDraw(m_pCommandBuffer);
        static_cast<CameraCtrl *>(m_pController)->drawHandles();
        Handles::endDraw();
    }
}

void Viewport::resizeGL(int width, int height) {
    SceneView::resizeGL(width, height);

    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->resize(width, height);
    }
}

void Viewport::dragEnterEvent(QDragEnterEvent *event) {
    emit dragEnter(event);
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
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}

void Viewport::keyReleaseEvent(QKeyEvent *pe) {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->onInputEvent(pe);
    }
}
