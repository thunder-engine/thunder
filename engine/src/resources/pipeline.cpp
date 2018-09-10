#include "pipeline.h"

#include <float.h>

#include "controller.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/scene.h>
#include <components/component.h>
#include <components/camera.h>
#include <components/directlight.h>

#include <resources/mesh.h>
#include <resources/rendertexture.h>

#include <analytics/profiler.h>
#include <log.h>

#include <commandbuffer.h>

#define SM_RESOLUTION_DEFAULT 1024
#define SM_RESOLUTION 2048

#define MAX_LODS 4

#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"
#define G_EMISSIVE  "emissiveMap"

#define SELECT_MAP  "selectMap"
#define DEPTH_MAP   "depthMap"
#define SHADOW_MAP  "shadowMap"

Pipeline::Pipeline() :
        m_Buffer(nullptr),
        m_Screen(Vector2(64, 64)),
        m_pSprite(nullptr) {

    m_Buffer    = Engine::objectCreate<ICommandBuffer>();

    Material *mtl   = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    if(mtl) {
        m_pSprite   = mtl->createInstance();
    }
    m_pPlane    = Engine::loadResource<Mesh>(".embedded/plane.fbx");

    RenderTexture *select   = Engine::objectCreate<RenderTexture>();
    select->setTarget(Texture::RGBA8);
    select->apply();
    m_Targets[SELECT_MAP]   = select;
    m_Buffer->setGlobalTexture(SELECT_MAP,  select);

    RenderTexture *depth    = Engine::objectCreate<RenderTexture>();
    depth->setDepth(24);
    depth->apply();
    m_Targets[DEPTH_MAP]    = depth;
    m_Buffer->setGlobalTexture(DEPTH_MAP,   depth);

    RenderTexture *shadow   = Engine::objectCreate<RenderTexture>();
    shadow->setDepth(24);
    depth->apply();
    m_Targets[SHADOW_MAP]   = shadow;
    m_Buffer->setGlobalTexture(SHADOW_MAP,  shadow);

    RenderTexture *normals  = Engine::objectCreate<RenderTexture>();
    normals->setTarget(Texture::RGB10A2);
    normals->apply();
    m_Targets[G_NORMALS]    = normals;
    m_Buffer->setGlobalTexture(G_NORMALS,   normals);

    RenderTexture *diffuse  = Engine::objectCreate<RenderTexture>();
    diffuse->setTarget(Texture::RGBA8);
    diffuse->apply();
    m_Targets[G_DIFFUSE]    = diffuse;
    m_Buffer->setGlobalTexture(G_DIFFUSE,   diffuse);

    RenderTexture *params   = Engine::objectCreate<RenderTexture>();
    params->setTarget(Texture::RGBA8);
    params->apply();
    m_Targets[G_PARAMS]     = params;
    m_Buffer->setGlobalTexture(G_PARAMS,    params);

    RenderTexture *emissive = Engine::objectCreate<RenderTexture>();
    emissive->setTarget(Texture::R11G11B10Float);
    emissive->apply();
    m_Targets[G_EMISSIVE]   = emissive;
    m_Buffer->setGlobalTexture(G_EMISSIVE,  emissive);

    shadow->resize(SM_RESOLUTION, SM_RESOLUTION);
    shadow->setFixed(true);

  //m_pBlur     = new ABlurGL();
  //m_pAO       = new AAmbientOcclusionGL();

  //m_PostEffects.push_back(new AAntiAliasingGL());
  //m_PostEffects.push_back(new ABloomGL());
}

Pipeline::~Pipeline() {
    for(auto it : m_Targets) {
        it.second->deleteLater();
    }
    m_Targets.clear();
}

void Pipeline::draw(Scene &scene, uint32_t resource) {
    m_ComponentList.clear();
    combineComponents(scene);

    // Light prepass
    m_Buffer->setGlobalValue("light.ambient", scene.ambient());

    m_Buffer->setRenderTarget(TargetBuffer(), m_Targets[SHADOW_MAP]);
    m_Buffer->clearRenderTarget();

    updateShadows(scene);

    m_Buffer->setViewport(0, 0, m_Screen.x, m_Screen.y);

    // Retrive object id
    m_Buffer->setRenderTarget({m_Targets[SELECT_MAP]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, Vector4(0.0));

    cameraReset();
    drawComponents(ICommandBuffer::RAYCAST);

    // Deffered shading

    Camera *camera  = static_cast<Camera *>(parent());
    // Fill G buffer pass
    m_Buffer->setRenderTarget({m_Targets[G_NORMALS], m_Targets[G_DIFFUSE], m_Targets[G_PARAMS], m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP], true);
    m_Buffer->clearRenderTarget(true, ((camera) ? camera->color() : Vector4(0.0)), false);

    cameraReset();
    // Draw Opaque pass
    drawComponents(ICommandBuffer::DEFAULT);

    // Screen Space Ambient Occlusion effect
    RenderTexture *t    = m_Targets[G_EMISSIVE];

    m_Buffer->setRenderTarget({t});

    // Light pass
    drawComponents(ICommandBuffer::LIGHT);

    cameraReset();
    // Draw Transparent pass
    drawComponents(ICommandBuffer::TRANSLUCENT);

    m_Buffer->setRenderTarget(resource);
    m_Buffer->setScreenProjection();

    m_pSprite->setTexture("texture0", postProcess(*t));
    m_Buffer->drawMesh(Matrix4(), m_pPlane, 0, ICommandBuffer::UI, m_pSprite);
}

void Pipeline::cameraReset() {
    Camera *camera  = static_cast<Camera *>(parent());
    if(camera) {
        Matrix4 v, p;
        camera->matrices(v, p);
        camera->setRatio(m_Screen.x / m_Screen.y);
        m_Buffer->setGlobalValue("camera.position", Vector4(camera->actor().transform()->position(), camera->nearPlane()));
        m_Buffer->setGlobalValue("camera.target", Vector4(Vector3(), camera->farPlane()));
        m_Buffer->setGlobalValue("camera.screen", Vector4(1.0f / m_Screen.x, 1.0f / m_Screen.y, m_Screen.x, m_Screen.y));
        m_Buffer->setGlobalValue("camera.mvpi", (p * v).inverse());
        m_Buffer->setGlobalValue("light.map", Vector4(1.0f / SM_RESOLUTION, 1.0f / SM_RESOLUTION, SM_RESOLUTION, SM_RESOLUTION));
        m_Buffer->setViewProjection(v, p);
    }
}

RenderTexture *Pipeline::target(const string &target) {
    auto it = m_Targets.find(target);
    if(it != m_Targets.end()) {
        return it->second;
    }
    return nullptr;
}

void Pipeline::resize(uint32_t width, uint32_t height) {
    m_Screen    = Vector2(width, height);

    for(auto &it : m_Targets) {
        it.second->resize(width, height);
    }
    for(auto &it : m_PostEffects) {
        //it->resize(width, height);
    }
}

void Pipeline::combineComponents(Object &object) {
    for(auto &it : object.getChildren()) {
        Object *child   = it;
        Component *draw = dynamic_cast<Component *>(child);
        if(draw) {
            if(draw->isEnable()) {
                m_ComponentList.push_back(draw);
            }
        } else {
            Actor *actor    = dynamic_cast<Actor *>(child);
            if(actor) {
                if(!actor->isEnable()) {
                    continue;
                }
            }
            combineComponents(*child);
        }
    }
}

void Pipeline::updateShadows(Object &object) {
    for(auto &it : object.getChildren()) {
        DirectLight *light = dynamic_cast<DirectLight *>(it);
        if(light) {
            directUpdate(light);
        } else {
            updateShadows(*it);
        }
    }
}

void Pipeline::drawComponents(uint32_t layer) {
    for(auto it : m_ComponentList) {
        it->draw(*m_Buffer, layer);
    }
}

void Pipeline::directUpdate(DirectLight *light) {
    Camera *camera  = static_cast<Camera *>(parent());
    if(camera) {
        Vector4 distance;
        Matrix4 mv, p;
        camera->matrices(mv, p);
        {
            float split     = 0.9f;
            float nearPlane = camera->nearPlane();
            float farPlane  = camera->farPlane();
            for(int i = 1; i <= MAX_LODS; i++) {
                float f = (float)i / (float)MAX_LODS;
                float l = nearPlane * pow(farPlane / nearPlane, f);
                float u = nearPlane + (farPlane - nearPlane) * f;
                float v = MIX(u, l, split);
                distance[i - 1] = v;
                Vector4 depth   = p * Vector4(0.0f, 0.0f, -v, 1.0f);
                light->normalizedDistance()[i - 1] = (depth.z / depth.w);
            }
        }

        float nearPlane = camera->nearPlane();
        Matrix4 view    = Matrix4(light->actor().transform()->rotation().toMatrix()).inverse();
        Matrix4 inv     = mv.inverse();
        for(uint32_t lod = 0; lod < MAX_LODS; lod++) {
            float dist  = distance[lod];
            Vector3 bb[2]   = {Vector3(FLT_MAX), Vector3(-FLT_MAX)};
            for(Vector3 &it : camera->frustumCorners(nearPlane, dist)) {
                Vector3 pos = (inv * it);
                bb[0].x = MIN(bb[0].x, pos.x);
                bb[0].y = MIN(bb[0].y, pos.y);
                bb[0].z = MIN(bb[0].z, pos.z);

                bb[1].x = MAX(bb[1].x, pos.x);
                bb[1].y = MAX(bb[1].y, pos.y);
                bb[1].z = MAX(bb[1].z, pos.z);
            }
            nearPlane   = dist;
            Matrix4 proj    = Matrix4::ortho(bb[0].x, bb[1].x,
                                             bb[0].y, bb[1].y,
                                             -100, 100);

            light->matrix()[lod]    = Matrix4(Vector3(0.5f), Quaternion(), Vector3(0.5f)) * proj * view;
            // Draw in the depth buffer from position of the light source
            m_Buffer->setViewProjection(view, proj);
            uint32_t x  = (lod % 2) * SM_RESOLUTION_DEFAULT;
            uint32_t y  = (lod / 2) * SM_RESOLUTION_DEFAULT;
            m_Buffer->setViewport(x, y, SM_RESOLUTION_DEFAULT, SM_RESOLUTION_DEFAULT);

            light->tiles()[lod] = Vector4((float)x / SM_RESOLUTION,
                                          (float)y / SM_RESOLUTION,
                                          (float)SM_RESOLUTION_DEFAULT / SM_RESOLUTION,
                                          (float)SM_RESOLUTION_DEFAULT / SM_RESOLUTION);

            drawComponents(ICommandBuffer::SHADOWCAST);
        }
    }
}

RenderTexture *Pipeline::postProcess(RenderTexture &source) {
    RenderTexture *result   = &source;
    for(auto it : m_PostEffects) {
        //result  = it->draw(*result, *m_Buffer);
    }
    return result;
}
