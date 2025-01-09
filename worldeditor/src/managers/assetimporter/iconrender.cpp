#include "iconrender.h"

#include <QImage>
#include <QWindow>

#include <engine.h>
#include <pipelinecontext.h>

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

#include <resources/texture.h>

IconRender::IconRender(QObject *parent) :
        QObject(parent),
        m_world(Engine::objectCreate<World>()),
        m_scene(Engine::objectCreate<Scene>("", m_world)),
        m_light(nullptr),
        m_render(nullptr),
        m_color(Engine::objectCreate<Texture>()) {

    m_actor = Engine::composeActor("Camera", "ActiveCamera", m_scene);
    m_actor->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_camera = static_cast<Camera *>(m_actor->component("Camera"));

    m_color->setFormat(Texture::RGBA8);
    m_color->resize(128, 128);
}

IconRender::~IconRender() {
    delete m_scene;
}

void IconRender::readPixels(void *object) {
    IconRender *ptr = reinterpret_cast<IconRender *>(object);
    if(ptr) {
        ptr->m_color->readPixels(0, 0, ptr->m_color->width(), ptr->m_color->height());
    }
}

const QImage IconRender::render(const QString &resource, const QString &) {
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

    if(m_render == nullptr) {
        m_render = PluginManager::instance()->createRenderer();
        m_render->init();
        PipelineContext *context = m_render->pipelineContext();
        if(context) {
            context->resize(m_color->width(), m_color->height());
            context->subscribePost(IconRender::readPixels, this);
        }

        m_light = Engine::composeActor("DirectLight", "LightSource", m_scene);
        m_light->transform()->setQuaternion(Vector3(-45.0f, 45.0f, 0.0f));
    }

    m_render->update(m_world);

    ByteArray data(m_color->getPixels(0));
    QImage result(data.data(), m_color->width(), m_color->height(), QImage::Format_RGBA8888);

    object->setParent(nullptr);
    delete object;

    return result.mirrored();
}
