#include "pipelinecontext.h"

#include "systems/rendersystem.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/world.h"
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
#include "pipelinepasses/translucent.h"
#include "pipelinepasses/guilayer.h"

#include "commandbuffer.h"

#include <algorithm>

#include <float.h>

#define OVERRIDE "texture0"

PipelineContext::PipelineContext() :
        m_buffer(Engine::objectCreate<CommandBuffer>()),
        m_postProcessSettings(new PostProcessSettings),
        m_finalMaterial(nullptr),
        m_defaultTarget(Engine::objectCreate<RenderTarget>()),
        m_final(nullptr),
        m_camera(nullptr),
        m_guiLayer(new GuiLayer),
        m_width(64),
        m_height(64),
        m_frustumCulling(true) {

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultPostEffect.shader");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
    }

    insertRenderPass(new ShadowMap);

    PipelinePass *gbuffer = new GBuffer;
    insertRenderPass(gbuffer);

    PipelinePass *occlusion = new AmbientOcclusion;
    occlusion->setInput(AmbientOcclusion::Input, gbuffer->output(GBuffer::Emissive));
    insertRenderPass(occlusion);

    PipelinePass *light = new DeferredLighting;
    light->setInput(DeferredLighting::Emissve, gbuffer->output(GBuffer::Emissive));
    insertRenderPass(light);

    PipelinePass *translucent = new Translucent;
    translucent->setInput(Translucent::Emissve, gbuffer->output(GBuffer::Emissive));
    translucent->setInput(Translucent::Depth, gbuffer->output(GBuffer::Depth));
    insertRenderPass(translucent);

    insertRenderPass(new Reflections);
    insertRenderPass(new AntiAliasing);
    insertRenderPass(new Bloom);
    insertRenderPass(m_guiLayer);
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

    // Finish
    m_buffer->setRenderTarget(m_defaultTarget);
    m_buffer->clearRenderTarget();

    m_finalMaterial->setTexture(OVERRIDE, m_final);
    m_buffer->drawMesh(Matrix4(), defaultPlane(), 0, CommandBuffer::UI, m_finalMaterial);
}

void PipelineContext::setCurrentCamera(Camera *camera) {
    m_camera = camera;
    m_camera->setRatio((float)m_width / (float)m_height);

    m_cameraView = m_camera->viewMatrix();
    m_cameraProjection = m_camera->projectionMatrix();
    Matrix4 vp = m_cameraProjection * m_cameraView;

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
    m_buffer->setGlobalValue("shadow.pageSize", Vector4(1.0f / size, 1.0f / size, size, size));
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

void PipelineContext::analizeGraph(World *world) {
    Camera *camera = Camera::current();
    Transform *cameraTransform = camera->transform();

    bool update = world->isToBeUpdated();

    // Add renderables
    m_sceneComponents.clear();
    for(auto it : RenderSystem::renderables()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor && actor->isEnabledInHierarchy()) {
                if((actor->world() == world)) {
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
        m_culledComponents = frustumCulling(Camera::frustumCorners(*camera), m_sceneComponents, m_worldBound);
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
                if((actor->world() == world)) {
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
                if((actor->world() == world)) {
                    if(update) {
                        static_cast<NativeBehaviour *>(it)->update();
                    }
                    m_uiComponents.push_back(it);
                }
            }
        }
    }

    // Add Post process volumes
    m_postProcessSettings->resetDefault();

    for(auto it : RenderSystem::postProcessVolumes()) {
        if(it->isEnabled()) {
            Actor *actor = it->actor();
            if(actor && actor->isEnabledInHierarchy()) {
                Scene *scene = actor->scene();
                if(scene && scene->parent() == world) {
                    if(!it->unbound() && !it->bound().intersect(cameraTransform->worldPosition(), camera->nearPlane())) {
                        continue;
                    }
                    m_postProcessSettings->lerp(it->settings(), it->blendWeight());
                }
            }
        }
    }

    for(auto &it : m_renderPasses) {
        it->setSettings(*m_postProcessSettings);
    }
}

RenderTarget *PipelineContext::defaultTarget() {
    return m_defaultTarget;
}

void PipelineContext::setDefaultTarget(RenderTarget *target) {
    m_defaultTarget = target;
}

void PipelineContext::addTextureBuffer(Texture *texture) {
    m_textureBuffers[texture->name()] = texture;
}

Texture *PipelineContext::textureBuffer(const string &string) {
    auto it = m_textureBuffers.find(string);
    if(it != m_textureBuffers.end()) {
        return it->second;
    }
    return nullptr;
}

Mesh *PipelineContext::defaultPlane() {
    static Mesh *plane = nullptr;
    if(plane == nullptr) {
        plane = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");
    }
    return plane;
}

Mesh *PipelineContext::defaultCube() {
    static Mesh *cube = nullptr;
    if(cube == nullptr) {
        cube = Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001");
    }
    return cube;
}

void PipelineContext::showUiAsSceneView() {
    m_guiLayer->showUiAsSceneView();
}

void PipelineContext::insertRenderPass(PipelinePass *pass, PipelinePass *before) {
    for(uint32_t i = 0; i < pass->outputCount(); i++) {
        Texture *texture = pass->output(i);
        m_buffer->setGlobalTexture(texture->name().c_str(), texture);
        addTextureBuffer(texture);
    }
    if(before) {
        auto it = std::find(m_renderPasses.begin(), m_renderPasses.end(), before);
        m_renderPasses.insert(it, pass);
    } else {
        m_renderPasses.push_back(pass);
    }
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
/*!
    Filters out an incoming \a list which are not in the \a frustum.
    Returns filtered list.
*/
list<Renderable *> PipelineContext::frustumCulling(const array<Vector3, 8> &frustum, list<Renderable *> &list, AABBox &bb) {
    bb.extent = Vector3(-1.0f);

    Plane pl[6];
    pl[0] = Plane(frustum[1], frustum[0], frustum[4]); // top
    pl[1] = Plane(frustum[7], frustum[3], frustum[2]); // bottom
    pl[2] = Plane(frustum[3], frustum[7], frustum[0]); // left
    pl[3] = Plane(frustum[2], frustum[1], frustum[6]); // right
    pl[4] = Plane(frustum[0], frustum[1], frustum[3]); // near
    pl[5] = Plane(frustum[5], frustum[4], frustum[6]); // far

    RenderList result;
    for(auto it : list) {
        AABBox box = it->bound();
        if(box.extent.x < 0.0f || box.intersect(pl, 6)) {
            result.push_back(it);
            bb.encapsulate(box);
        }
    }
    return result;
}

AABBox PipelineContext::worldBound() const {
    return m_worldBound;
}
