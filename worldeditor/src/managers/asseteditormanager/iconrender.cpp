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
#include <editor/pluginmanager.h>

#include <systems/rendersystem.h>

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
        float radius = (bb.radius * 2.0f) / sinf(m_pCamera->fov() * DEG2RAD);
        Vector3 cameraPosition(bb.center + m_pActor->transform()->quaternion() * Vector3(0.0, 0.0, radius));
        m_pActor->transform()->setPosition(cameraPosition);
    } else {
        return QImage();
    }

    Camera::setCurrent(m_pCamera);
    ByteArray data = PluginManager::instance()->render()->renderOffscreen(m_pScene, 128, 128);

    delete object;

    QImage result((uint8_t *)data.data(), 128, 128, QImage::Format_RGBA8888);

    return result.mirrored();
}
