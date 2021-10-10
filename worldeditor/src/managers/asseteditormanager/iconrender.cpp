#include "iconrender.h"

#include <QImage>

#include <engine.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/directlight.h>

#include <resources/pipeline.h>

#include <editor/assetconverter.h>
#include <systems/rendersystem.h>

#include "pluginmanager.h"
#include "assetmanager.h"

IconRender::IconRender(Engine *engine, QObject *parent) :
        QObject(parent),
        m_Init(false),
        m_pLight(nullptr) {

    m_pScene = Engine::objectCreate<Scene>();
    m_pActor = Engine::composeActor("Camera", "ActiveCamera", m_pScene);
    m_pActor->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_pCamera = static_cast<Camera *>(m_pActor->component("Camera"));
}

IconRender::~IconRender() {
    delete m_pScene;
}

const QImage IconRender::render(const QString &resource, const QString &) {
    if(!m_Init) {
        Pipeline *pipe = m_pCamera->pipeline();
        pipe->resize(128, 128);

        m_pLight = Engine::composeActor("DirectLight", "LightSource", m_pScene);
        m_pLight->transform()->setQuaternion(Vector3(-45.0f, 45.0f, 0.0f));

        m_Init = true;
    }

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

        m_pCamera->setOrthographic(false);
        float fov = m_pCamera->fov();
        m_pActor->transform()->setPosition(Vector3(bb.center.x, bb.center.y,
                                                 ((bb.extent.length() + m_pCamera->nearPlane()) * 1.1f) / sinf(fov * DEG2RAD)));
    } else {
        return QImage();
    }

    Camera::setCurrent(m_pCamera);
    vector<uint8_t> data = PluginManager::instance()->render()->renderOffscreen(m_pScene, 128, 128);

    delete object;

    return QImage();
}
