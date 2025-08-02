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
        m_color(nullptr) {

    m_actor = Engine::composeActor("Camera", "ActiveCamera", m_scene);
    m_actor->transform()->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_camera = m_actor->getComponent<Camera>();
    m_camera->setOrthographic(true);
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

const QImage IconRender::render(const TString &uuid) {
    Camera::setCurrent(m_camera);

    Actor *object = AssetManager::instance()->createActor(uuid);
    if(object) {
        object->setParent(m_scene);

        AABBox bb(0.0f, -1.0f);
        for(auto it : object->findChildren<Renderable *>()) {
            bb.encapsulate(it->bound());
        }

        m_camera->transform()->setPosition(bb.center + Vector3(0.0f, 0.0f, bb.radius));
        m_camera->setOrthoSize(bb.radius * 2.0f);
        m_camera->setFar(bb.radius * 2.0f);
    } else {
        return QImage();
    }

    if(m_render == nullptr) {
        m_render = PluginManager::instance()->createRenderer();
        m_render->init();
        PipelineContext *context = m_render->pipelineContext();
        if(context) {
            m_color = context->resultTexture();
            m_color->setFlags(m_color->flags() | Texture::Feedback);

            context->resize(128, 128);
            context->subscribePost(IconRender::readPixels, this);
        }

        m_light = Engine::composeActor("DirectLight", "LightSource", m_scene);
        m_light->transform()->setQuaternion(Vector3(-45.0f, 45.0f, 0.0f));
    }

    m_render->update(m_world);

    if(m_color) {
        ByteArray data(m_color->getPixels(0));
        QImage result(data.data(), m_color->width(), m_color->height(), QImage::Format_RGBA8888);

        object->setParent(nullptr);
        delete object;

        return result.mirrored();
    }

    return QImage();
}
