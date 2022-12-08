#include "pipelinecontext.h"

#include "systems/rendersystem.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/scenegraph.h"
#include "components/camera.h"
#include "components/renderable.h"
#include "components/baselight.h"
#include "components/postprocessvolume.h"
#include "components/gui/widget.h"

#include "components/private/postprocessorsettings.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinepasses/gbuffer.h"
#include "pipelinepasses/ambientocclusion.h"
#include "pipelinepasses/antialiasing.h"
#include "pipelinepasses/reflections.h"
#include "pipelinepasses/bloom.h"
#include "pipelinepasses/shadowmap.h"
#include "pipelinepasses/deferredlighting.h"

#include "commandbuffer.h"

#include <algorithm>

#include <float.h>

#define OVERRIDE "texture0"

bool typeLessThan(PostProcessVolume *left, PostProcessVolume *right) {
    return left->priority() < right->priority();
}

PipelineContext::PipelineContext() :
        m_buffer(Engine::objectCreate<CommandBuffer>()),
        m_finalMaterial(nullptr),
        m_defaultTarget(Engine::objectCreate<RenderTarget>()),
        m_camera(nullptr),
        m_final(nullptr),
        m_debugTexture(nullptr),
        m_width(64),
        m_height(64),
        m_uiAsSceneView(false),
        m_frustumCulling(true) {

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultPostEffect.shader");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
    }

    addRenderPass(new ShadowMap);

    PipelinePass *gbuffer = new GBuffer;
    addRenderPass(gbuffer);

    PipelinePass *occlusion = new AmbientOcclusion;
    occlusion->setInput(AmbientOcclusion::Input, gbuffer->output(GBuffer::Emissive));
    addRenderPass(occlusion);

    PipelinePass *light = new DeferredLighting;
    light->setInput(DeferredLighting::Emissve, gbuffer->output(GBuffer::Emissive));
    light->setInput(DeferredLighting::Depth, gbuffer->output(GBuffer::Depth));
    addRenderPass(light);

    addRenderPass(new Reflections);
    addRenderPass(new AntiAliasing);
    addRenderPass(new Bloom); /// \todo Should it be before or after AA?
}

PipelineContext::~PipelineContext() {
    m_textureBuffers.clear();
}

CommandBuffer *PipelineContext::buffer() const {
    return m_buffer;
}

void PipelineContext::draw(Camera *camera) {
    setCurrentCamera(camera);

    m_final = nullptr;
    for(auto it : m_renderPasses) {
        if(it->isEnabled()) {
            m_final = it->draw(m_final, this);
        }
    }

    // UI pass
    if(!m_uiAsSceneView) {
        m_buffer->setScreenProjection(0, 0, m_width, m_height);
    }
    for(auto it : m_uiComponents) {
        it->draw(*m_buffer, CommandBuffer::UI);
    }

    // Finish
    m_buffer->setRenderTarget(m_defaultTarget);
    m_buffer->clearRenderTarget();

    m_finalMaterial->setTexture(OVERRIDE, (m_debugTexture != nullptr) ? m_debugTexture : m_final);
    m_buffer->drawMesh(Matrix4(), defaultPlane(), 0, CommandBuffer::UI, m_finalMaterial);
}

void PipelineContext::setCurrentCamera(Camera *camera) {
    m_camera = camera;

    m_cameraView = m_camera->viewMatrix();
    m_cameraProjection = m_camera->projectionMatrix();
    Matrix4 vp = m_cameraProjection * m_cameraView;

    m_camera->setRatio((float)m_width / (float)m_height);

    Transform *c = m_camera->transform();

    m_buffer->setGlobalValue("camera.position", Vector4(c->worldPosition(), m_camera->nearPlane()));
    m_buffer->setGlobalValue("camera.target", Vector4(c->worldTransform().rotation() * Vector3(0.0f, 0.0f, 1.0f), m_camera->farPlane()));
    m_buffer->setGlobalValue("camera.view", m_cameraView);
    m_buffer->setGlobalValue("camera.projectionInv", m_cameraProjection.inverse());
    m_buffer->setGlobalValue("camera.projection", m_cameraProjection);
    m_buffer->setGlobalValue("camera.screenToWorld", vp.inverse());
    m_buffer->setGlobalValue("camera.worldToScreen", vp);
}

void PipelineContext::cameraReset() {
    m_buffer->setViewProjection(m_cameraView, m_cameraProjection);
}

void PipelineContext::setMaxTexture(uint32_t size) {
    m_buffer->setGlobalValue("light.pageSize", Vector4(1.0f / size, 1.0f / size, size, size));
}

void PipelineContext::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        m_width = width;
        m_height = height;

        for(auto &it : m_renderPasses) {
            it->resize(m_width, m_height);
        }

        m_buffer->setGlobalValue("camera.screen", Vector4(1.0f / (float)m_width, 1.0f / (float)m_height, m_width, m_height));
    }
}

void PipelineContext::analizeGraph(SceneGraph *graph) {
    Camera *camera = Camera::current();
    Transform *cameraTransform = camera->transform();

    bool update = graph->isToBeUpdated();

    // Add renderables
    m_sceneComponents.clear();
    for(auto it : RenderSystem::renderables()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor && actor->isEnabledInHierarchy()) {
                if((actor->scene() && actor->scene()->parent() == graph)) {
                    if(update) {
                        it->update();
                    }
                    m_sceneComponents.push_back(it);
                }
            }
        }
    }
    // Renderables cull and sort
    if(m_frustumCulling) {
        m_culledComponents = Camera::frustumCulling(m_sceneComponents, Camera::frustumCorners(*camera));
    }

    Vector3 origin = cameraTransform->position();
    culledComponents().sort([origin](const Renderable *left, const Renderable *right) {
        int p1 = left->priority();
        int p2 = right->priority();
        if(p1 == p2) {
            const Matrix4 &m1 = left->transform()->worldTransform();
            const Matrix4 &m2 = right->transform()->worldTransform();

            return origin.dot(Vector3(m1[12], m1[13], m1[14])) < origin.dot(Vector3(m2[12], m2[13], m2[14]));
        }
        return p1 < p2;
    });

    // Add lights
    m_sceneLights.clear();
    for(auto it : RenderSystem::lights()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor && actor->isEnabledInHierarchy()) {
                if((actor->scene() && actor->scene()->parent() == graph)) {
                    m_sceneLights.push_back(it);
                }
            }
        }
    }

    // Add widgets
    m_uiComponents.clear();
    for(auto it : RenderSystem::widgets()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor && actor->isEnabledInHierarchy()) {
                if((actor->scene() && actor->scene()->parent() == graph)) {
                    if(update) {
                        static_cast<NativeBehaviour *>(it)->update();
                    }
                    m_uiComponents.push_back(it);
                }
            }
        }
    }

    // Add Post process volumes
    PostProcessSettings &settings = graph->finalPostProcessSettings();
    settings.resetDefault();

    m_postProcessVolume.clear();
    for(auto it : RenderSystem::postProcessVolumes()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor->isEnabledInHierarchy()) {
                if((actor->scene() && actor->scene()->parent() == graph)) {
                    m_postProcessVolume.push_back(it);

                    if(!it->unbound()) {
                        if(!it->bound().intersect(cameraTransform->worldPosition(), camera->nearPlane())) {
                            continue;
                        }
                    }
                    settings.lerp(it->settings(), it->blendWeight());
                }
            }
        }
    }

    m_buffer->setGlobalValue("light.ambient", 0.1f/*settings.ambientLightIntensity()*/);
    for(auto &it : m_renderPasses) {
        it->setSettings(settings);
    }
}

RenderTarget *PipelineContext::defaultTarget() {
    return m_defaultTarget;
}

void PipelineContext::setDefaultTarget(RenderTarget *target) {
    m_defaultTarget = target;
}

Texture *PipelineContext::textureBuffer(const string &string) {
    auto it = m_textureBuffers.find(string);
    if(it != m_textureBuffers.end()) {
        return it->second;
    }
    return nullptr;
}

Texture *PipelineContext::debugTexture() const {
    return m_debugTexture;
}

void PipelineContext::setDebugTexture(const string &string) {
    m_debugTexture = nullptr;
    auto it = m_textureBuffers.find(string);
    if(it != m_textureBuffers.end()) {
        m_debugTexture = it->second;
    }
}

Mesh *PipelineContext::defaultPlane() {
    static Mesh *plane = nullptr;
    if(plane == nullptr) {
        plane = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");
    }
    return plane;
}

void PipelineContext::showUiAsSceneView() {
    m_uiAsSceneView = true;
}

void PipelineContext::addRenderPass(PipelinePass *pass) {
    for(uint32_t i = 0; i < pass->outputCount(); i++) {
        Texture *texture = pass->output(i);
        m_buffer->setGlobalTexture(texture->name().c_str(), texture);
        m_textureBuffers[texture->name()] = texture;
    }
    m_renderPasses.push_back(pass);
}

const list<PipelinePass *> &PipelineContext::renderPasses() const {
    return m_renderPasses;
}

list<string> PipelineContext::renderTextures() const {
    list<string> result;
    for(auto &it : m_textureBuffers) {
        result.push_back(it.first);
    }

    return result;
}

void PipelineContext::drawRenderers(uint32_t layer, const list<Renderable *> &list) {
    for(auto it : list) {
        it->draw(*m_buffer, layer);
    }
}

list<Renderable *> &PipelineContext::sceneComponents() {
    return m_sceneComponents;
}

list<Renderable *> &PipelineContext::culledComponents() {
    return m_frustumCulling ? m_culledComponents : m_sceneComponents;
}

list<BaseLight *> &PipelineContext::sceneLights() {
    return m_sceneLights;
}

list<Widget *> &PipelineContext::uiComponents() {
    return m_uiComponents;
}

Camera *PipelineContext::currentCamera() const {
    return m_camera;
}
