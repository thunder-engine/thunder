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
#include "pipelinetasks/indirect.h"
#include "pipelinetasks/shadowmap.h"
#include "pipelinetasks/translucent.h"
#include "pipelinetasks/tonemap.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

int32_t RenderSystem::m_registered = 0;

std::list<BaseLight *> RenderSystem::m_lightComponents;
std::list<Renderable *> RenderSystem::m_renderableComponents;
std::list<PostProcessVolume *> RenderSystem::m_postProcessVolumes;

RenderSystem::RenderSystem() :
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
        DeferredIndirect::registerClassFactory(this);
        ShadowMap::registerClassFactory(this);
        Translucent::registerClassFactory(this);
        Tonemap::registerClassFactory(this);
    }
    ++m_registered;

    setName("Render");
}

RenderSystem::~RenderSystem() {
    --m_registered;

    if(m_registered) {
        // Core
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

        PostProcessVolume::unregisterClassFactory(this);

        // System
        CommandBuffer::unregisterClassFactory(this);

        PipelineContext::unregisterClassFactory(this);

        // Pipline tasks
        PipelineTask::unregisterClassFactory(this);
        AmbientOcclusion::unregisterClassFactory(this);
        AntiAliasing::unregisterClassFactory(this);
        Bloom::unregisterClassFactory(this);
        DeferredLighting::unregisterClassFactory(this);
        GBuffer::unregisterClassFactory(this);
        Reflections::unregisterClassFactory(this);
        DeferredIndirect::unregisterClassFactory(this);
        ShadowMap::unregisterClassFactory(this);
        Translucent::unregisterClassFactory(this);
        Tonemap::unregisterClassFactory(this);
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
#endif
