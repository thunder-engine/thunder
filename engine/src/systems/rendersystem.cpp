#include "systems/rendersystem.h"

#include "components/meshrender.h"
#include "components/textrender.h"
#include "components/spriterender.h"
#include "components/particlerender.h"
#include "components/directlight.h"
#include "components/pointlight.h"
#include "components/spotlight.h"
#include "components/arealight.h"
#include "components/skinnedmeshrender.h"

#include "components/postprocesssettings.h"

#include "components/camera.h"

#include "resources/pipeline.h"

#include "commandbuffer.h"

class RenderSystemPrivate {
public:
    static int32_t m_AtlasPageWidth;
    static int32_t m_AtlasPageHeight;
};

int32_t RenderSystemPrivate::m_AtlasPageWidth = 1024;
int32_t RenderSystemPrivate::m_AtlasPageHeight = 1024;

static bool classesRegistered = false;

RenderSystem::RenderSystem() :
        p_ptr(new RenderSystemPrivate()) {

    if(!classesRegistered) {
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

        ICommandBuffer::registerClassFactory(this);

        PostProcessSettings::registerClassFactory(this);

        classesRegistered = true;
    }
}

RenderSystem::~RenderSystem() {
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

    ICommandBuffer::unregisterClassFactory(this);

    PostProcessSettings::unregisterClassFactory(this);
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
/*!
    Main drawing procedure.
*/
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

void RenderSystem::atlasPageSize(int32_t &width, int32_t &height) {
    width = RenderSystemPrivate::m_AtlasPageWidth;
    height = RenderSystemPrivate::m_AtlasPageHeight;
}

void RenderSystem::setAtlasPageSize(int32_t width, int32_t height) {
    RenderSystemPrivate::m_AtlasPageWidth = width;
    RenderSystemPrivate::m_AtlasPageHeight = height;
}
