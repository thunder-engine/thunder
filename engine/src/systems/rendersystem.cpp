#include "systems/rendersystem.h"

#include "components/world.h"
#include "components/meshrender.h"
#include "components/textrender.h"
#include "components/spriterender.h"
#include "components/effectrender.h"
#include "components/directlight.h"
#include "components/pointlight.h"
#include "components/spotlight.h"
#include "components/arealight.h"
#include "components/skinnedmeshrender.h"
#include "components/tilemaprender.h"

#include "components/postprocessvolume.h"

#include "components/camera.h"
#include "components/actor.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinetasks/ambientocclusion.h"
#include "pipelinetasks/antialiasing.h"
#include "pipelinetasks/bloom.h"
#include "pipelinetasks/deferredlighting.h"
#include "pipelinetasks/gbuffer.h"
#include "pipelinetasks/reflections.h"
#include "pipelinetasks/shadowmap.h"
#include "pipelinetasks/translucent.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

int32_t RenderSystem::m_registered = 0;

std::list<BaseLight *> RenderSystem::m_lightComponents;
std::list<Renderable *> RenderSystem::m_renderableComponents;
std::list<PostProcessVolume *> RenderSystem::m_postProcessVolumes;

RenderSystem::RenderSystem() :
        m_offscreen(false),
        m_pipelineContext(nullptr) {

    if(m_registered == 0) {
        // Core
        Renderable::registerClassFactory(this);
        MeshRender::registerClassFactory(this);
        TextRender::registerClassFactory(this);
        SpriteRender::registerClassFactory(this);
        SkinnedMeshRender::registerClassFactory(this);

        BaseLight::registerClassFactory(this);
        DirectLight::registerClassFactory(this);
        PointLight::registerClassFactory(this);
        SpotLight::registerClassFactory(this);
        AreaLight::registerClassFactory(this);

        EffectRender::registerClassFactory(this);

        TileMapRender::registerClassFactory(this);

        PostProcessVolume::registerClassFactory(this);

        // System
        CommandBuffer::registerClassFactory(this);

        PipelineContext::registerClassFactory(this);

        // Pipline tasks
        PipelineTask::registerClassFactory(this);
        AmbientOcclusion::registerClassFactory(this);
        AntiAliasing::registerClassFactory(this);
        Bloom::registerClassFactory(this);
        DeferredLighting::registerClassFactory(this);
        GBuffer::registerClassFactory(this);
        Reflections::registerClassFactory(this);
        ShadowMap::registerClassFactory(this);
        Translucent::registerClassFactory(this);
    }
    ++m_registered;

    setName("Render");
}

RenderSystem::~RenderSystem() {
    --m_registered;

    if(m_registered) {
        Renderable::unregisterClassFactory(this);
        MeshRender::unregisterClassFactory(this);
        TextRender::unregisterClassFactory(this);
        SpriteRender::unregisterClassFactory(this);
        SkinnedMeshRender::unregisterClassFactory(this);

        BaseLight::unregisterClassFactory(this);
        DirectLight::unregisterClassFactory(this);
        PointLight::unregisterClassFactory(this);
        SpotLight::unregisterClassFactory(this);
        AreaLight::unregisterClassFactory(this);

        EffectRender::unregisterClassFactory(this);

        TileMapRender::unregisterClassFactory(this);

        CommandBuffer::unregisterClassFactory(this);

        PostProcessVolume::unregisterClassFactory(this);
    }
}

int RenderSystem::threadPolicy() const {
    return Main;
}

bool RenderSystem::init() {
    m_pipelineContext = Engine::objectCreate<PipelineContext>("PipelineContext");
    return true;
}

void RenderSystem::update(World *world) {
    PROFILE_FUNCTION();

    Camera *camera = Camera::current();
    if(camera && m_pipelineContext) {
        PROFILER_RESET(POLYGONS);
        PROFILER_RESET(DRAWCALLS);

        m_pipelineContext->setWorld(world);
        m_pipelineContext->draw(camera);
    }
}

void RenderSystem::composeComponent(Component *component) const {
    component->composeComponent();
}

PipelineContext *RenderSystem::pipelineContext() const {
    return m_pipelineContext;
}

void RenderSystem::setOffscreenMode(bool mode) {
    m_offscreen = mode;
}

bool RenderSystem::isOffscreenMode() const {
    return m_offscreen;
}

void RenderSystem::addRenderable(Renderable *renderable) {
    m_renderableComponents.push_back(renderable);
}

void RenderSystem::removeRenderable(Renderable *renderable) {
    m_renderableComponents.remove(renderable);
}

std::list<Renderable *> &RenderSystem::renderables() {
    return m_renderableComponents;
}

void RenderSystem::addLight(BaseLight *light) {
    m_lightComponents.push_back(light);
}
void RenderSystem::removeLight(BaseLight *light) {
    m_lightComponents.remove(light);
}

std::list<BaseLight *> &RenderSystem::lights() {
    return m_lightComponents;
}

void RenderSystem::addPostProcessVolume(PostProcessVolume *volume) {
    m_postProcessVolumes.push_back(volume);
}

void RenderSystem::removePostProcessVolume(PostProcessVolume *volume) {
    m_postProcessVolumes.remove(volume);
}

std::list<PostProcessVolume *> &RenderSystem::postProcessVolumes() {
    return m_postProcessVolumes;
}

#if defined(SHARED_DEFINE)
QWindow *RenderSystem::createRhiWindow() {
    return nullptr;
}

ByteArray RenderSystem::renderOffscreen(World *sceneGraph, int width, int height) {
    static Texture *color = nullptr;
    if(color == nullptr) {
        color = Engine::objectCreate<Texture>();
        color->setFormat(Texture::RGBA8);
    }
    color->resize(width, height);

    static Texture *depth = nullptr;
    if(depth == nullptr) {
        depth = Engine::objectCreate<Texture>();
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
    }
    depth->resize(width, height);

    static RenderTarget *target = nullptr;
    if(target == nullptr) {
        target = Engine::objectCreate<RenderTarget>();
        target->setColorAttachment(0, color);
        target->setDepthAttachment(depth);
    }
    RenderTarget *back = nullptr;

    Camera *camera = Camera::current();
    if(camera && m_pipelineContext) {
        m_pipelineContext->resize(width, height);

        back = m_pipelineContext->defaultTarget();
        m_pipelineContext->setDefaultTarget(target);
    }

    setOffscreenMode(true);
    update(sceneGraph);
    setOffscreenMode(false);

    color->readPixels(0, 0, width, height);

    if(m_pipelineContext && back) {
        m_pipelineContext->setDefaultTarget(back);
    }

    return color->getPixels(0);
}
#endif
