#include "iconrender.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

#include <QImage>
#include <QFileInfo>

#include <engine.h>

#include <system.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/directlight.h>
#include <components/meshrender.h>
#include <components/spriterender.h>

#include <resources/pipeline.h>
#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/sprite.h>
#include <resources/prefab.h>

#include <editor/converter.h>

#include "pluginmanager.h"
#include "assetmanager.h"
#include "controllers/cameractrl.h"

IconRender::IconRender(Engine *engine, QOpenGLContext *share, QObject *parent) :
        QObject(parent),
        m_Init(false) {

    m_pEngine = engine;

    m_Surface = new QOffscreenSurface();
    m_Surface->create();

    m_Context = new QOpenGLContext();
    m_Context->setShareContext(share);
    m_Context->setFormat(m_Surface->requestedFormat());
    m_Context->create();
    m_Context->makeCurrent(m_Surface);

    m_pFBO = new QOpenGLFramebufferObject(128, 128);

    m_pScene = Engine::objectCreate<Scene>();
    m_pActor = Engine::composeActor("Camera", "ActiveCamera", m_pScene);
    m_pActor->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_pCamera = static_cast<Camera *>(m_pActor->component("Camera"));

    m_pLight = nullptr;

    m_pController = nullptr;
}

IconRender::~IconRender() {
    delete m_pScene;
}

const QImage IconRender::render(const QString &resource, const QString &) {
    m_Context->makeCurrent(m_Surface);

    if(!m_Init) {
        Pipeline *pipe = m_pCamera->pipeline();
        pipe->resize(m_pFBO->size().width(), m_pFBO->size().height());
        pipe->setTarget(m_pFBO->handle());

        m_pLight = Engine::composeActor("DirectLight", "LightSource", m_pScene);
        Matrix3 rot;
        rot.rotate(Vector3(-45.0f, 45.0f, 0.0f));
        m_pLight->transform()->setQuaternion(rot);

        m_Init = true;
    }

    m_pCamera->setOrthographic(false);
    float fov = m_pCamera->fov();

    Actor *object = AssetManager::instance()->createActor(resource);
    if(object) {
        object->setParent(m_pScene);

        AABBox bb;
        bool first = true;
        for(auto it : object->findChildren<Renderable *>()) {
            if(first) {
                bb = it->bound();
                first = false;
            } else {
                bb.encapsulate(it->bound());
            }
        }

        m_pActor->transform()->setPosition(Vector3(bb.center.x, bb.center.y, ((bb.extent.length() + m_pCamera->nearPlane()) * 1.1f) / sinf(fov * DEG2RAD)));
    } else {
        return QImage();
    }

    Camera::setCurrent(m_pCamera);
    PluginManager::instance()->updateRender(m_pScene);
    PluginManager::instance()->updateRender(m_pScene);

    m_Context->functions()->glFinish();

    delete object;
    return m_pFBO->toImage();
}
