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
#include "pipelinetasks/depthoffield.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

void *RenderSystem::m_windowHandle = nullptr;

RenderSystem::RenderSystem() :
        m_pipelineContext(nullptr),
        m_frameDirty(true) {

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
    DepthOfField::registerClassFactory(this);

    setName("RenderSystem");
}

RenderSystem::~RenderSystem() {
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
    DepthOfField::unregisterClassFactory(this);
}

int RenderSystem::threadPolicy() const {
    return Main;
}

bool RenderSystem::init() {
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

PipelineContext *RenderSystem::pipelineContext() const {
    return m_pipelineContext;
}

void RenderSystem::setPipelineContext(PipelineContext *context) {
    m_pipelineContext = context;
}

#if defined(SHARED_DEFINE)
QWindow *RenderSystem::createRhiWindow(Viewport *viewport) {
    return nullptr;
}
#endif

void *RenderSystem::windowHandle() {
    return m_windowHandle;
}

void RenderSystem::setWindowHandle(void *handle) {
    m_windowHandle = handle;
}
