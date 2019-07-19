#include "pipeline.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/scene.h"
#include "components/camera.h"
#include "components/directlight.h"
#include "components/meshrender.h"
#include "components/particlerender.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertexture.h"

#include "analytics/profiler.h"
#include "log.h"

#include "commandbuffer.h"

#include <algorithm>

#include <float.h>

#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"
#define G_EMISSIVE  "emissiveMap"

#define DEPTH_MAP   "depthMap"
#define SHADOW_MAP  "shadowMap"

#define OVERRIDE "uni.texture0"

Pipeline::Pipeline() :
        m_Buffer(nullptr),
        m_Screen(Vector2(64, 64)),
        m_pSprite(nullptr),
        m_Target(0) {

    m_Buffer    = Engine::objectCreate<ICommandBuffer>();

    Material *mtl   = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    if(mtl) {
        m_pSprite   = mtl->createInstance();
    }
    m_pPlane    = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    RenderTexture *depth    = Engine::objectCreate<RenderTexture>();
    depth->setDepth(24);
    m_Targets[DEPTH_MAP]    = depth;
    m_Buffer->setGlobalTexture(DEPTH_MAP,   depth);

    RenderTexture *shadow   = Engine::objectCreate<RenderTexture>();
    shadow->setDepth(24);
    shadow->resize(SM_RESOLUTION, SM_RESOLUTION);
    shadow->setFixed(true);
    m_Targets[SHADOW_MAP]   = shadow;
    m_Buffer->setGlobalTexture(SHADOW_MAP,  shadow);

    RenderTexture *normals  = Engine::objectCreate<RenderTexture>();
    normals->setTarget(Texture::RGB10A2);
    m_Targets[G_NORMALS]    = normals;
    m_Buffer->setGlobalTexture(G_NORMALS,   normals);

    RenderTexture *diffuse  = Engine::objectCreate<RenderTexture>();
    diffuse->setTarget(Texture::RGBA8);
    m_Targets[G_DIFFUSE]    = diffuse;
    m_Buffer->setGlobalTexture(G_DIFFUSE,   diffuse);

    RenderTexture *params   = Engine::objectCreate<RenderTexture>();
    params->setTarget(Texture::RGBA8);
    m_Targets[G_PARAMS]     = params;
    m_Buffer->setGlobalTexture(G_PARAMS,    params);

    RenderTexture *emissive = Engine::objectCreate<RenderTexture>();
    emissive->setTarget(Texture::R11G11B10Float);
    m_Targets[G_EMISSIVE]   = emissive;
    m_Buffer->setGlobalTexture(G_EMISSIVE,  emissive);

  //m_pBlur     = new ABlurGL();
  //m_pAO       = new AAmbientOcclusionGL();

  //m_PostEffects.push_back(new AAntiAliasingGL());
  //m_PostEffects.push_back(new ABloomGL());
}

Pipeline::~Pipeline() {
    m_Targets.clear();
}

void Pipeline::draw(Scene *scene, Camera &camera) {
    Transform *t = camera.actor()->transform();
    ObjectList filter = Camera::frustumCulling(m_Components, Camera::frustumCorners(camera.orthographic(), (camera.orthographic()) ? camera.orthoHeight() : camera.fov(),
                                                                                    camera.ratio(), t->worldPosition(), t->worldRotation(), camera.nearPlane(), camera.farPlane()));
    sortByDistance(filter, camera.actor()->transform()->position());

    updateShadows(camera, scene);

    // Light prepass
    m_Buffer->setGlobalValue("light.ambient", scene->ambient());

    cameraReset(camera);

    // Step1 - Fill G buffer pass
    m_Buffer->setRenderTarget({m_Targets[G_NORMALS], m_Targets[G_DIFFUSE], m_Targets[G_PARAMS], m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, camera.color());

    // Draw Opaque pass
    drawComponents(ICommandBuffer::DEFAULT, filter);

    /// \todo Screen Space Ambient Occlusion effect should be defined here
    m_Buffer->setRenderTarget({m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP]);

    // Step2 - Light pass
    drawComponents(ICommandBuffer::LIGHT, filter);

    cameraReset(camera);
    // Step3 - Draw Transparent pass
    drawComponents(ICommandBuffer::TRANSLUCENT, filter);

    m_Buffer->setRenderTarget(m_Target);
    m_Buffer->setScreenProjection();
    m_Buffer->clearRenderTarget(true, Vector4());

    m_pSprite->setTexture(OVERRIDE, postProcess(*m_Targets[G_EMISSIVE]));
    m_Buffer->drawMesh(Matrix4(), m_pPlane, ICommandBuffer::UI, m_pSprite);
}

void Pipeline::cameraReset(Camera &camera) {
    Matrix4 v, p;
    camera.matrices(v, p);
    camera.setRatio(m_Screen.x / m_Screen.y);
    m_Buffer->setGlobalValue("camera.position", Vector4(camera.actor()->transform()->worldPosition(), camera.nearPlane()));
    m_Buffer->setGlobalValue("camera.target", Vector4(Vector3(), camera.farPlane()));
    m_Buffer->setGlobalValue("camera.screen", Vector4(1.0f / m_Screen.x, 1.0f / m_Screen.y, m_Screen.x, m_Screen.y));
    m_Buffer->setGlobalValue("camera.mvpi", (p * v).inverse());
    m_Buffer->setGlobalValue("light.map", Vector4(1.0f / SM_RESOLUTION, 1.0f / SM_RESOLUTION, SM_RESOLUTION, SM_RESOLUTION));
    m_Buffer->setViewProjection(v, p);
}

RenderTexture *Pipeline::target(const string &target) const {
    auto it = m_Targets.find(target);
    if(it != m_Targets.end()) {
        return it->second;
    }
    return nullptr;
}

Mesh *Pipeline::plane() const {
    return m_pPlane;
}

MaterialInstance *Pipeline::sprite() const {
    return m_pSprite;
}

void Pipeline::resize(uint32_t width, uint32_t height) {
    m_Screen    = Vector2(width, height);

    for(auto &it : m_Targets) {
        it.second->resize(width, height);
    }
    //for(auto &it : m_PostEffects) {
    //    //it->resize(width, height);
    //}
}

void Pipeline::combineComponents(Object *object, bool first) {
    if(first) {
        m_Components.clear();
    }
    for(auto &it : object->getChildren()) {
        Object *child = it;
        Renderable *comp = dynamic_cast<Renderable *>(child);
        if(comp) {
            if(comp->isEnabled()) {
                m_Components.push_back(comp);
            }
        } else {
            Actor *actor = dynamic_cast<Actor *>(child);
            if(actor) {
                if(!actor->isEnabled()) {
                    continue;
                }
            }
            combineComponents(child);
        }
    }
}

void Pipeline::setTarget(uint32_t resource) {
    m_Target = resource;
}

void Pipeline::drawComponents(uint32_t layer, ObjectList &list) {
    for(auto it : list) {
        static_cast<Renderable *>(it)->draw(*m_Buffer, layer);
    }
}

void Pipeline::updateShadows(Camera &camera, Object *object) {
    m_Buffer->setRenderTarget(TargetBuffer(), m_Targets[SHADOW_MAP]);

    for(auto &it : object->getChildren()) {
        BaseLight *light = dynamic_cast<BaseLight *>(it);
        if(light) {
            light->shadowsUpdate(camera, *m_Buffer, m_Components);
        } else {
            updateShadows(camera, it);
        }
    }

    m_Buffer->setViewport(0, 0, static_cast<int32_t>(m_Screen.x), static_cast<int32_t>(m_Screen.y));
}

RenderTexture *Pipeline::postProcess(RenderTexture &source) {
    RenderTexture *result   = &source;
    //for(auto it : m_PostEffects) {
    //    result  = it->draw(*result, *m_Buffer);
    //}
    return result;
}

struct ObjectComp {
    bool operator() (const Object *left, const Object *right) {
        if(dynamic_cast<const DirectLight *>(left)) {
            return false;
        } else {
            Matrix4 m1 = static_cast<const Component *>(left)->actor()->transform()->worldTransform();
            Matrix4 m2 = static_cast<const Component *>(right)->actor()->transform()->worldTransform();
            return origin.dot(Vector3(m1[12], m1[13], m1[14])) < origin.dot(Vector3(m2[12], m2[13], m2[14]));
        }
    }
    Vector3 origin;
};

void Pipeline::sortByDistance(ObjectList &in, const Vector3 &origin) {
    ObjectComp comp;
    comp.origin = origin;

    in.sort(comp);
}
