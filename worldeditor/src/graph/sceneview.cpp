#include <QApplication>
#include <QMimeData>
#include <QDragEnterEvent>

#include <QPainter>

#include "sceneview.h"

#include <engine.h>
#include <system.h>
#include <controller.h>
#include <components/scene.h>
#include <components/chunk.h>
#include <components/camera.h>

#include "common.h"
#include "managers/pluginmanager/pluginmodel.h"

SceneView::SceneView(QWidget *parent) :
        QOpenGLWidget(parent),
        m_pController(nullptr),
        m_pRender(nullptr),
        m_pScene(nullptr),
        m_RenderDesc("RenderGL"),
        m_GameMode(false) {

    setMouseTracking(true);
}

SceneView::~SceneView() {
    delete m_pRender;
}

void SceneView::setRender(const QString &render) {
    m_RenderDesc    = render;
}

void SceneView::setScene(Scene *scene) {
    if(m_pScene) {
        m_pScene->deleteLater();
    }
    m_pScene    = scene;
    PluginModel::instance()->addScene(m_pScene);
}

void SceneView::setController(IController *ctrl) {
    m_pController   = ctrl;
}

void SceneView::startGame() {
    m_pScene->start();

    m_GameMode  = true;
}

void SceneView::stopGame() {
    m_GameMode  = false;
}

bool SceneView::isGame() const {
    return m_GameMode;
}

void SceneView::initializeGL() {
    if(!m_RenderDesc.isEmpty()) {
        m_pRender   = PluginModel::instance()->createSystem(qPrintable(m_RenderDesc));
    }
    if(m_pRender) {
        m_pRender->init();
        if(m_pController) {
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
        findCamera();
        m_pScene->update();
        if(m_pRender) {
            m_pRender->update(*m_pScene, defaultFramebufferObject());
        }
    }
}

void SceneView::resizeGL(int width, int height) {
    QOpenGLWidget::resizeGL(width, height);

    if(m_pRender) {
        m_pRender->resize(width, height);
    }
}

void SceneView::findCamera() {
    Chunk *chunk    = m_pScene->findChild<Chunk *>();
    if(chunk && m_pController) {
        m_pController->setActiveCamera(chunk->findChild<Camera *>());
    }
}
