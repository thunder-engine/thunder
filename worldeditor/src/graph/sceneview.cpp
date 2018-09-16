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
#include "pluginmodel.h"

SceneView::SceneView(QWidget *parent) :
        QOpenGLWidget(parent),
        m_pController(nullptr),
        m_pScene(nullptr),
        m_GameMode(false) {

    setMouseTracking(true);
}

SceneView::~SceneView() {
    foreach(ISystem *it, m_Systems) {
        delete it;
    }
    m_Systems.clear();
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
    m_Systems.push_back(PluginModel::instance()->createSystem("Media"));
    m_Systems.push_back(PluginModel::instance()->createSystem("RenderGL"));

    foreach(ISystem *it, m_Systems) {
        if(it) {
            it->init();
            if(m_pController) {
                it->overrideController(m_pController);
            }
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
        uint32_t handle = defaultFramebufferObject();
        foreach(ISystem *it, m_Systems) {
            if(it) {
                it->resize(width(), height());
                it->update(*m_pScene, handle);
            }
        }
    }
}

void SceneView::resizeGL(int width, int height) {
    QOpenGLWidget::resizeGL(width, height);

    foreach(ISystem *it, m_Systems) {
        if(it) {
            it->resize(width, height);
        }
    }
}

void SceneView::findCamera() {
    Chunk *chunk    = m_pScene->findChild<Chunk *>();
    if(chunk && m_pController) {
        m_pController->setActiveCamera(chunk->findChild<Camera *>());
    }
}
