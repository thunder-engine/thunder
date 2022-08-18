#include "systems/rendersystem.h"

#include "components/scenegraph.h"
#include "components/meshrender.h"
#include "components/textrender.h"
#include "components/spriterender.h"
#include "components/particlerender.h"
#include "components/directlight.h"
#include "components/pointlight.h"
#include "components/spotlight.h"
#include "components/arealight.h"
#include "components/skinnedmeshrender.h"

#include "components/postprocessvolume.h"

#include "components/camera.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

class RenderSystemPrivate {
public:
    RenderSystemPrivate() :
        m_offscreen(false),
        m_pipelineContext(nullptr) {

    }
    static int32_t m_AtlasPageWidth;
    static int32_t m_AtlasPageHeight;

    static int32_t m_registered;

    bool m_offscreen;

    PipelineContext *m_pipelineContext;
};

int32_t RenderSystemPrivate::m_AtlasPageWidth = 1024;
int32_t RenderSystemPrivate::m_AtlasPageHeight = 1024;

int32_t RenderSystemPrivate::m_registered = 0;

RenderSystem::RenderSystem() :
        p_ptr(new RenderSystemPrivate()) {

    if(RenderSystemPrivate::m_registered == 0) {
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

        ParticleRender::registerClassFactory(this);

        CommandBuffer::registerClassFactory(this);

        PostProcessVolume::registerClassFactory(this);

        PipelineContext::registerClassFactory(this);
    }
    ++RenderSystemPrivate::m_registered;
}

RenderSystem::~RenderSystem() {
    --RenderSystemPrivate::m_registered;

    if(RenderSystemPrivate::m_registered) {
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

        ParticleRender::unregisterClassFactory(this);

        CommandBuffer::unregisterClassFactory(this);

        PostProcessVolume::unregisterClassFactory(this);
    }
}

int RenderSystem::threadPolicy() const {
    return Main;
}

const char *RenderSystem::name() const {
    return "Render";
}

bool RenderSystem::init() {
    p_ptr->m_pipelineContext = new PipelineContext;
    return true;
}

void RenderSystem::update(SceneGraph *sceneGraph) {
    PROFILE_FUNCTION();

    PROFILER_RESET(POLYGONS);
    PROFILER_RESET(DRAWCALLS);

    Camera *camera = Camera::current();
    if(camera && p_ptr->m_pipelineContext) {
        p_ptr->m_pipelineContext->analizeScene(sceneGraph, this);
        p_ptr->m_pipelineContext->drawMain(*camera);
        p_ptr->m_pipelineContext->drawUi(*camera);
        p_ptr->m_pipelineContext->finish();
    }
}

void RenderSystem::atlasPageSize(int32_t &width, int32_t &height) {
    width = RenderSystemPrivate::m_AtlasPageWidth;
    height = RenderSystemPrivate::m_AtlasPageHeight;
}

void RenderSystem::setAtlasPageSize(int32_t width, int32_t height) {
    RenderSystemPrivate::m_AtlasPageWidth = width;
    RenderSystemPrivate::m_AtlasPageHeight = height;
}

void RenderSystem::composeComponent(Component *component) const {
    Renderable *renderable = dynamic_cast<Renderable *>(component);
    if(renderable) {
        renderable->composeComponent();
    }
}

PipelineContext *RenderSystem::pipelineContext() const {
    return p_ptr->m_pipelineContext;
}

void RenderSystem::setOffscreenMode(bool mode) {
    p_ptr->m_offscreen = mode;
}

bool RenderSystem::isOffscreenMode() const {
    return p_ptr->m_offscreen;
}

#if defined(SHARED_DEFINE)
QWindow *RenderSystem::createRhiWindow() const {
    return nullptr;
}

ByteArray RenderSystem::renderOffscreen(SceneGraph *sceneGraph, int width, int height) {
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
    if(camera && p_ptr->m_pipelineContext) {
        p_ptr->m_pipelineContext->resize(width, height);

        back = p_ptr->m_pipelineContext->defaultTarget();
        p_ptr->m_pipelineContext->setDefaultTarget(target);
    }

    setOffscreenMode(true);
    update(sceneGraph);
    setOffscreenMode(false);

    color->readPixels(0, 0, width, height);

    if(p_ptr->m_pipelineContext && back) {
        p_ptr->m_pipelineContext->setDefaultTarget(back);
    }

    return color->getPixels(0);
}
#endif
