#include "systems/rendersystem.h"

#include "components/scene.h"
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

#include "resources/pipeline.h"
#include "resources/material.h"

#include "commandbuffer.h"

#define DEFAULTSPRITE ".embedded/DefaultSprite.mtl"

class RenderSystemPrivate {
public:
    RenderSystemPrivate() :
        m_Update(true) {

    }
    static int32_t m_AtlasPageWidth;
    static int32_t m_AtlasPageHeight;

    bool m_Update;
};

int32_t RenderSystemPrivate::m_AtlasPageWidth = 1024;
int32_t RenderSystemPrivate::m_AtlasPageHeight = 1024;

RenderSystem::RenderSystem() :
        p_ptr(new RenderSystemPrivate()) {

}

RenderSystem::~RenderSystem() {

}

void RenderSystem::registerClasses() {
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
}

void RenderSystem::unregisterClasses() {
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

int RenderSystem::threadPolicy() const {
    return Main;
}

const char *RenderSystem::name() const {
    return "Render";
}

bool RenderSystem::init() {
    return true;
}

void RenderSystem::update(Scene *scene) {
    PROFILE_FUNCTION();

    PROFILER_RESET(POLYGONS);
    PROFILER_RESET(DRAWCALLS);

    Camera *camera = Camera::current();
    if(camera) {
        Pipeline *pipe = camera->pipeline();
        pipe->analizeScene(scene, this);
        pipe->draw(*camera);
        pipe->finish();
    }
}

void RenderSystem::processEvents() {
    m_pScene->setToBeUpdated(true);
    System::processEvents();
    m_pScene->setToBeUpdated(false);
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
    SpriteRender *sprite = dynamic_cast<SpriteRender *>(component);
    if(sprite) {
        sprite->setMaterial(Engine::loadResource<Material>(DEFAULTSPRITE));
    }
}
#if defined(NEXT_SHARED)
QWindow *RenderSystem::createRhiWindow() const {
    return nullptr;
}
#endif
