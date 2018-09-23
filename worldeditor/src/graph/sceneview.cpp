#include <QApplication>
#include <QMimeData>
#include <QDragEnterEvent>

#include <QPainter>

#include "sceneview.h"

#include <engine.h>
#include <system.h>
#include <controller.h>
#include <components/actor.h>
#include <components/scene.h>
#include <components/camera.h>

#include <resources/pipeline.h>

#include "common.h"
#include "pluginmodel.h"

SceneView::SceneView(QWidget *parent) :
        QOpenGLWidget(parent),
        m_pController(nullptr),
        m_pScene(nullptr),
        m_GameMode(false),
        m_MouseButtons(0) {

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
        if(m_GameMode) {
            updateScene(m_pScene);
        }

        findCamera();

        uint32_t handle = defaultFramebufferObject();
        foreach(ISystem *it, m_Systems) {
            if(it) {
                it->update(*m_pScene, handle);
            }
        }
    }
}

void SceneView::resizeGL(int width, int height) {
    QOpenGLWidget::resizeGL(width, height);

    if(m_pController) {
        Camera *camera  = m_pController->activeCamera();
        if(camera) {
            camera->pipeline()->resize(width, height);
        }
    }
}

void SceneView::mousePressEvent(QMouseEvent *ev) {
    m_MouseButtons  = ev->buttons();
}

void SceneView::mouseReleaseEvent(QMouseEvent *ev) {
    m_MouseButtons  = ev->buttons();
}

void SceneView::updateScene(Object *object) {
    Engine::updateScene(object);
}

void SceneView::findCamera() {
    Actor *chunk    = m_pScene->findChild<Actor *>(false);
    if(chunk && m_pController) {
        Camera *camera  = chunk->findChild<Camera *>();
        if(camera) {
            camera->pipeline()->resize(width(), height());
            m_pController->setActiveCamera(camera);
        }
    }
}
