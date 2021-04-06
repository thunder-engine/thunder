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

    RenderList m_Lights;
    RenderList m_Objects;
    list<PostProcessSettings *> m_Settings;
};

int32_t RenderSystemPrivate::m_AtlasPageWidth = 1024;
int32_t RenderSystemPrivate::m_AtlasPageHeight = 1024;

RenderSystem::RenderSystem() :
        p_ptr(new RenderSystemPrivate()) {
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

list<Renderable *> &RenderSystem::renderable() const {
    return p_ptr->m_Objects;
}

list<Renderable *> &RenderSystem::lights() const {
    return p_ptr->m_Lights;
}

list<PostProcessSettings *> &RenderSystem::postPcessSettings() const {
    return p_ptr->m_Settings;
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

Object *RenderSystem::instantiateObject(const MetaObject *meta) {
    Object *result = System::instantiateObject(meta);
    if(dynamic_cast<Renderable *>(result)) {
        if(dynamic_cast<BaseLight *>(result)) {
            p_ptr->m_Lights.push_back(static_cast<Renderable *>(result));
        } else {
            p_ptr->m_Objects.push_back(static_cast<Renderable *>(result));
        }
    } else if(dynamic_cast<PostProcessSettings *>(result)) {
        p_ptr->m_Settings.push_back(static_cast<PostProcessSettings *>(result));
    }
    return result;
}

void RenderSystem::removeObject(Object *object) {
    System::removeObject(object);

    p_ptr->m_Lights.remove(static_cast<Renderable *>(object));
    p_ptr->m_Objects.remove(static_cast<Renderable *>(object));
    p_ptr->m_Settings.remove(static_cast<PostProcessSettings *>(object));
}

void RenderSystem::atlasPageSize(int32_t &width, int32_t &height) {
    width = RenderSystemPrivate::m_AtlasPageWidth;
    height = RenderSystemPrivate::m_AtlasPageHeight;
}

void RenderSystem::setAtlasPageSize(int32_t width, int32_t height) {
    RenderSystemPrivate::m_AtlasPageWidth = width;
    RenderSystemPrivate::m_AtlasPageHeight = height;
}
