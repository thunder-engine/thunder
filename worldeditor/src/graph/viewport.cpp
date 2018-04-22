#include "viewport.h"

#include <system.h>
#include <commandbuffer.h>

#include "handles.h"
#include "controllers/objectctrl.h"

Viewport::Viewport(QWidget *parent) :
        SceneView(parent),
        m_pCommandBuffer(new ICommandBuffer())  {

    setAutoFillBackground(false);

}

void Viewport::addButton(OverlayButton *button) {
    int width   = 10;
    foreach(OverlayButton *it, m_OverlayButtons) {
        width   += it->rect().width() + gRoundness;
    }
    button->setPos(QPoint(width, 10));
    m_OverlayButtons.push_back(button);
}

void Viewport::initializeGL() {
    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->init(m_pScene);
    }
    SceneView::initializeGL();
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

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawOverlay(painter);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
}

void Viewport::resizeGL(int width, int height) {
    SceneView::resizeGL(width, height);

    if(m_pController) {
        static_cast<CameraCtrl *>(m_pController)->resize(width, height);
    }
}

void Viewport::drawOverlay(QPainter &painter) {
    painter.setPen(Qt::white);

    foreach(OverlayButton *it, m_OverlayButtons) {
        it->draw(painter);
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
    foreach(OverlayButton *it, m_OverlayButtons) {
        it->onMouseEvent(pe);
    }

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
    foreach(OverlayButton *it, m_OverlayButtons) {
        if(it->onMouseEvent(pe)) {
            return;
        }
    }

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
