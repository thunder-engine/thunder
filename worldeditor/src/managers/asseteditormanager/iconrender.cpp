#include "iconrender.h"

#include <QImage>

#include <engine.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/scenegraph.h>
#include <components/camera.h>
#include <components/directlight.h>

#include <editor/assetconverter.h>
#include <editor/pluginmanager.h>

#include <systems/rendersystem.h>

#include "assetmanager.h"

IconRender::IconRender(QObject *parent) :
        QObject(parent),
        m_init(false),
        m_light(nullptr) {

    m_scene = Engine::objectCreate<SceneGraph>();
    m_actor = Engine::composeActor("Camera", "ActiveCamera", m_scene);
    m_actor->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_camera = static_cast<Camera *>(m_actor->component("Camera"));
}

IconRender::~IconRender() {
    delete m_scene;
}

const QImage IconRender::render(const QString &resource, const QString &) {
    if(!m_init) {
        m_light = Engine::composeActor("DirectLight", "LightSource", m_scene);
        m_light->transform()->setQuaternion(Vector3(-45.0f, 45.0f, 0.0f));

        m_init = true;
    }

    Actor *object = AssetManager::instance()->createActor(resource);
    if(object) {
        object->setParent(m_scene);

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

        m_camera->setOrthographic(false);
        float radius = (bb.radius * 2.0f) / sinf(m_camera->fov() * DEG2RAD);
        Vector3 cameraPosition(bb.center + m_actor->transform()->quaternion() * Vector3(0.0, 0.0, radius));
        m_actor->transform()->setPosition(cameraPosition);
    } else {
        return QImage();
    }

    Camera::setCurrent(m_camera);
    RenderSystem *render = PluginManager::instance()->render();
    ByteArray data = render->renderOffscreen(m_scene, 128, 128);

    delete object;

    QImage result((uint8_t *)data.data(), 128, 128, QImage::Format_RGBA8888);

    return result.mirrored();
}
