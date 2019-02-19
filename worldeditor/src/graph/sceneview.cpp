#include <QApplication>
#include <QMimeData>
#include <QDragEnterEvent>

#include <QPainter>

#include "sceneview.h"

#include <engine.h>
#include <system.h>

#include <components/actor.h>
#include <components/scene.h>
#include <components/camera.h>

#include <resources/pipeline.h>

#include "controllers/cameractrl.h"

#include "pluginmodel.h"

SceneView::SceneView(QWidget *parent) :
        QOpenGLWidget(parent),
        m_pController(nullptr),
        m_pScene(nullptr),
        m_MouseButtons(0) {

    setMouseTracking(true);
}

SceneView::~SceneView() {

}

void SceneView::setScene(Scene *scene) {
    if(m_pScene) {
        m_pScene->deleteLater();
    }
    m_pScene    = scene;
    PluginModel::instance()->addScene(m_pScene);
}

void SceneView::setController(CameraCtrl *ctrl) {
    m_pController   = ctrl;
}

void SceneView::initializeGL() {
    emit inited();
}

void SceneView::paintGL() {
    if(m_pController) {
        m_pController->update();
    }
    if(m_pScene) {
        findCamera();

        PluginModel::instance()->updateSystems(m_pScene);
    }
}

void SceneView::resizeGL(int width, int height) {
    QOpenGLWidget::resizeGL(width, height);

    if(m_pController) {
        Camera *camera  = Camera::current();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width, height);
            pipe->setTarget(defaultFramebufferObject());
        }
    }
}

void SceneView::mousePressEvent(QMouseEvent *ev) {
    m_MouseButtons  = ev->buttons();
}

void SceneView::mouseReleaseEvent(QMouseEvent *ev) {
    m_MouseButtons  = ev->buttons();
}

void SceneView::findCamera() {
    Actor *chunk    = m_pScene->findChild<Actor *>(false);
    if(chunk) {
        Camera *camera  = chunk->findChild<Camera *>();
        if(camera) {
            Pipeline *pipe = camera->pipeline();
            pipe->resize(width(), height());
            pipe->setTarget(defaultFramebufferObject());
        }
        Camera::setCurrent(camera);
    }
}
