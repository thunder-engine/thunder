#include <QApplication>
#include <QMimeData>
#include <QDragEnterEvent>

#include <QPainter>

#include "sceneview.h"

#include <engine.h>
#include <rendersystem.h>
#include <components/scene.h>
#include <components/camera.h>

#include "common.h"
#include "controllers/objectctrl.h"

#include "managers/pluginmanager/pluginmodel.h"

#include "handles.h"

SceneView::SceneView(Engine *engine, QWidget *parent, QGLWidget *share) :
        QGLWidget(parent, share),
        m_pController(nullptr),
        m_pRender(nullptr),
        m_pScene(nullptr),
        m_RenderDesc("RenderGL") {

    m_pEngine   = engine;

    setFormat(QGLFormat(QGL::SampleBuffers));
    setAutoFillBackground(false);

    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    setMouseTracking(true);
    //setContextMenuPolicy(Qt::CustomContextMenu);
}

SceneView::~SceneView() {
    delete m_pRender;
}

void SceneView::setRender(const QString &render) {
    m_RenderDesc    = render;
}

void SceneView::setController(CameraCtrl *ctrl) {
    m_pController   = ctrl;
}

void SceneView::addButton(OverlayButton *button) {
    int width   = 10;
    foreach(OverlayButton *it, m_OverlayButtons) {
        width   += it->rect().width() + gRoundness;
    }
    button->setPos(QPoint(width, 10));
    m_OverlayButtons.push_back(button);
}

void SceneView::initializeGL() {
    PluginModel *plugin = PluginModel::instance();

    m_pScene    = Engine::objectCreate<Scene>("Scene");
    plugin->addScene(m_pScene);

    if(!m_RenderDesc.isEmpty()) {
        m_pRender   = static_cast<IRenderSystem *>(plugin->createSystem(qPrintable(m_RenderDesc)));
    }
    if(m_pRender) {
        m_pRender->init();
        Handles::init(m_pRender);
        if(m_pController) {
            m_pController->init(m_pScene);
            m_pRender->overrideController(m_pController);
        }
    }

    emit inited();
}

void SceneView::paintGL() {
    if(m_pController) {
        m_pController->update();
    }
    if(m_pScene) {
        m_pScene->update();
        if(m_pRender) {
            m_pRender->update(*m_pScene);
        }
    }
    Handles::s_ActiveCamera = m_pController->activeCamera();
    Handles::beginDraw();
    if(m_pController) {
        m_pController->drawHandles();
    }
    Handles::endDraw();
}

void SceneView::resizeGL(int width, int height) {
    if(m_pController) {
        Camera *c   =  m_pController->activeCamera();
        if(c) {
            c->resize(width, height);
        }
    }
}

void SceneView::paintEvent(QPaintEvent *event) {
    makeCurrent();

    paintGL();

    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    drawOverlay(painter);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
    painter.end();
}

void SceneView::drawOverlay(QPainter &painter) {
    painter.setPen(Qt::white);

    foreach(OverlayButton *it, m_OverlayButtons) {
        it->draw(painter);
    }
}

void SceneView::dragEnterEvent(QDragEnterEvent *event) {
    emit dragEnter(event);
}

void SceneView::dragLeaveEvent(QDragLeaveEvent *event) {
    emit dragLeave(event);
}

void SceneView::dropEvent(QDropEvent *event) {
    setFocus();
    emit drop(event);
}

void SceneView::mouseMoveEvent(QMouseEvent *pe) {
    foreach(OverlayButton *it, m_OverlayButtons) {
        it->onMouseEvent(pe);
    }

    if(m_pController) {
        m_pController->onInputEvent(pe);
    }
}

void SceneView::mousePressEvent(QMouseEvent *pe) {
    if(m_pController) {
        m_pController->onInputEvent(pe);
    }
}

void SceneView::mouseReleaseEvent(QMouseEvent *pe) {
    foreach(OverlayButton *it, m_OverlayButtons) {
        if(it->onMouseEvent(pe)) {
            return;
        }
    }

    if(m_pController) {
        m_pController->onInputEvent(pe);
    }
}

void SceneView::wheelEvent(QWheelEvent *pe) {
    if(m_pController) {
        m_pController->onInputEvent(pe);
    }
}

void SceneView::keyPressEvent(QKeyEvent *pe) {
    if(m_pController) {
        m_pController->onInputEvent(pe);
    }
}

void SceneView::keyReleaseEvent(QKeyEvent *pe) {
    if(m_pController) {
        m_pController->onInputEvent(pe);
    }
}
