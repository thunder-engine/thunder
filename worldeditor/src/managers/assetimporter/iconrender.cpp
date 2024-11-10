#include "iconrender.h"

#include <QImage>

#include <engine.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/world.h>
#include <components/scene.h>
#include <components/camera.h>
#include <components/directlight.h>

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>
#include <editor/pluginmanager.h>

#include <systems/rendersystem.h>

IconRender::IconRender(QObject *parent) :
        QObject(parent),
        m_world(Engine::objectCreate<World>()),
        m_scene(Engine::objectCreate<Scene>("", m_world)),
        m_init(false),
        m_light(nullptr) {

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
    Camera::setCurrent(m_camera);

    Actor *object = AssetManager::instance()->createActor(resource);
    if(object) {
        object->setParent(m_scene);

        AABBox bb(0.0f, -1.0f);
        for(auto it : object->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }

        m_camera->setOrthographic(false);
        float radius = (bb.radius * 2.0f) / sinf(m_camera->fov() * DEG2RAD);
        Vector3 cameraPosition(bb.center + m_actor->transform()->quaternion() * Vector3(0.0, 0.0, radius));
        m_actor->transform()->setPosition(cameraPosition);
    } else {
        return QImage();
    }

    static RenderSystem *render = nullptr;
    if(render == nullptr) {
        render = PluginManager::instance()->createRenderer();
        render->init();
    }
    ByteArray data = render->renderOffscreen(m_world, 128, 128);
    QImage result(reinterpret_cast<uint8_t *>(data.data()), 128, 128, QImage::Format_RGBA8888);

    object->setParent(nullptr);
    delete object;

    return result.mirrored();
}
