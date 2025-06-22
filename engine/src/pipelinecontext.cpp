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

#include "components/private/postprocessorsettings.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"
#include "resources/pipeline.h"

#include "pipelinetask.h"
#include "commandbuffer.h"
#include "log.h"

#include <algorithm>

#include <float.h>

#include "frustum.h"

namespace {
    const char *gTexture("mainTexture");
};

/*!
    \class PipelineContext
    \brief Class responsible for managing the rendering pipeline context.
    \inmodule Engine

    PipelineContext is a class responsible for managing the rendering pipeline context, including rendering tasks, camera settings, and post-processing effects.
*/

PipelineContext::PipelineContext() :
        m_world(nullptr),
        m_pipeline(nullptr),
        m_buffer(Engine::objectCreate<CommandBuffer>()),
        m_finalMaterial(nullptr),
        m_defaultTarget(Engine::objectCreate<RenderTarget>("defaultTarget")),
        m_camera(nullptr),
        m_width(64),
        m_height(64),
        m_frustumCulling(true),
        m_renderablesSorting(true) {

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultPostEffect.shader");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
    }

    setPipeline(Engine::loadResource<Pipeline>(Engine::value(".pipeline", ".embedded/Deferred.pipeline").toString()));

    uint32_t size = Texture::maxTextureSize();
    m_buffer->setGlobalValue("shadow.pageSize", Vector4(1.0f / size, 1.0f / size, size, size));
}

PipelineContext::~PipelineContext() {
    m_textureBuffers.clear();

    m_defaultTarget->deleteLater();
    m_buffer->deleteLater();

    delete m_finalMaterial;
}
/*!
    Retrieves the command buffer associated with the pipeline context.
*/
CommandBuffer *PipelineContext::buffer() const {
    return m_buffer;
}
/*!
    \internal
    Initiates the rendering process using the specified camera.
*/
void PipelineContext::draw(Camera *camera) {
    analizeGraph();

    camera->setRatio((float)m_width / (float)m_height);
    setCurrentCamera(camera);

    for(auto it : m_renderTasks) {
        if(it && it->isEnabled()) {
            it->exec();
        }
    }

    m_finalMaterial->setTexture(gTexture, resultTexture());

    // Finish
    m_buffer->setRenderTarget(m_defaultTarget);
    m_buffer->drawMesh(defaultPlane(), 0, CommandBuffer::UI, *m_finalMaterial);

    for(auto it : m_postObservers) {
        (*it.first)(it.second);
    }
}
/*!
    Sets the current \a camera and updates associated matrices in the command buffer.
*/
void PipelineContext::setCurrentCamera(Camera *camera) {
    m_camera = camera;

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
/*!
     Resets the camera view and projection matrices in the command buffer.
*/
void PipelineContext::cameraReset() {
    m_buffer->setViewProjection(m_cameraView, m_cameraProjection);
}
/*!
    Resizes the pipeline context to the specified \a width and \a height. Updates render tasks accordingly.
*/
void PipelineContext::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        m_width = width;
        m_height = height;

        for(auto &it : m_renderTasks) {
            it->resize(m_width, m_height);
        }

        m_buffer->setGlobalValue("camera.screen", Vector4(1.0f / (float)m_width, 1.0f / (float)m_height, m_width, m_height));
    }
}
/*!
    Invalidates all the pipeline tasks to let them to reestablish connections.
*/
void PipelineContext::invalidateTasks() {
    for(int i = 0; i < m_pipeline->renderTasksLinksCount(); i++) {
        Pipeline::Link link = m_pipeline->renderTaskLink(i);
        PipelineTask *output = nullptr;
        PipelineTask *input = nullptr;
        for(auto it : m_renderTasks) {
            if(output && input) {
                break;
            }
            if(it->name() == link.source) {
                output = it;
            }
            if(it->name() == link.target) {
                input = it;
            }
        }

        if(output && input) {
            input->setInput(link.input, output->output(link.output));
        } else {
            aError() << "Unable to link" << link.source.c_str() << "and" << link.target.c_str();
        }
    }
}
/*!
    Subscribes \a callback for \a object to handle post rendering step.
*/
void PipelineContext::subscribePost(RenderCallback callback, void *object) {
    m_postObservers.push_back(std::make_pair(callback, object));
}
/*!
    Unsubscribes an \a object to stop handle post rendering step.
*/
void PipelineContext::unsubscribePost(void *object) {
    auto it = m_postObservers.begin();
    while(it != m_postObservers.end()) {
        if((it->second) == object) {
            it = m_postObservers.erase(it);
        } else {
            ++it;
        }
    }
}
/*!
    \internal
    Analyzes the scene graph to determine which components and lights are relevant for rendering. Updates post-process settings.
*/
void PipelineContext::analizeGraph() {
    Camera *camera = Camera::current();
    Transform *cameraTransform = camera->transform();

    bool update = m_world->isToBeUpdated();

    // Add renderables
    m_sceneComponents.clear();
    for(auto it : RenderSystem::renderables()) {
        if(it->world() == m_world && it->isEnabledInHierarchy()) {
            if(update) {
                it->update();
            }
            m_sceneComponents.push_back(it);
        }
    }
    // Renderables frustum culling
    if(m_frustumCulling) {
         frustumCulling(camera->frustum(), m_sceneComponents, m_culledComponents, &m_worldBound);
    }

    // Renderables sort
    if(m_renderablesSorting) {
        Vector3 origin(cameraTransform->worldPosition());
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
    }

    // Add lights
    m_sceneLights.clear();
    for(auto it : RenderSystem::lights()) {
        if(it->world() == m_world && it->isEnabledInHierarchy()) {
            m_sceneLights.push_back(it);
        }
    }

    // Add Post process volumes
    m_culledPostProcessSettings.clear();
    for(auto it : RenderSystem::postProcessVolumes()) {
        if(it->world() == m_world && it->isEnabledInHierarchy()) {
            if(!it->unbound() && !it->bound().intersect(cameraTransform->worldPosition(), camera->nearPlane())) {
                continue;
            }
            m_culledPostProcessSettings.push_back(std::make_pair(it->settings(), it->blendWeight()));
        }
    }

    for(auto &it : m_renderTasks) {
        it->analyze(m_world);
    }
}
/*!
    Returns the curent world instance to process.
*/
World *PipelineContext::world() {
    return m_world;
}
/*!
    Sets the curent \a world instance to process.
*/
void PipelineContext::setWorld(World *world) {
    m_world = world;
}
/*!
    Returns the resulting texture containing the rendering result.
*/
Texture *PipelineContext::resultTexture() {
    if(!m_renderTasks.empty()) {
        return m_renderTasks.back()->output(0);
    }
    return nullptr;
}
/*!
    Returns the default render target associated with the pipeline context.
*/
RenderTarget *PipelineContext::defaultTarget() {
    return m_defaultTarget;
}
/*!
    Sets the default render \a target for the pipeline context.
*/
void PipelineContext::setDefaultTarget(RenderTarget *target) {
    m_defaultTarget = target;
}
/*!
    Adds a \a texture buffer to the global textures in the command buffer.
*/
void PipelineContext::addTextureBuffer(Texture *texture) {
    m_buffer->setGlobalTexture(texture->name().c_str(), texture);
    m_textureBuffers[texture->name()] = texture;
}
/*!
    Returns a texture buffer based on its \a name.
*/
Texture *PipelineContext::textureBuffer(const std::string &name) {
    auto it = m_textureBuffers.find(name);
    if(it != m_textureBuffers.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Retrieves the default plane mesh used in rendering.
*/
Mesh *PipelineContext::defaultPlane() {
    static Mesh *plane = nullptr;
    if(plane == nullptr) {
        plane = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");
        if(plane) {
            plane->incRef();
        }
    }
    return plane;
}
/*!
    Return the default cube mesh used in rendering.
*/
Mesh *PipelineContext::defaultCube() {
    static Mesh *cube = nullptr;
    if(cube == nullptr) {
        cube = Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001");
        cube->incRef();
    }
    return cube;
}
/*!
    Sets the rendering \a pipeline for the context, creating and linking associated rendering tasks.
*/
void PipelineContext::setPipeline(Pipeline *pipeline) {
    m_pipeline = pipeline;

    if(m_pipeline) {
        for(int i = 0; i < m_pipeline->renderTasksCount(); i++) {
            std::string taskName = m_pipeline->renderTaskName(i);
            PipelineTask *task = dynamic_cast<PipelineTask *>(Engine::objectCreate(taskName, taskName, this));
            if(task) {
                insertRenderTask(task);
            } else {
                aError() << "Wrong Pipeline Task type:" << taskName.c_str();
            }
        }

        invalidateTasks();
    }
}
/*!
     Inserts a rendering \a task into the pipeline context. Optionally, specifies the task to insert \a before.
*/
void PipelineContext::insertRenderTask(PipelineTask *task, PipelineTask *before) {
    if(task) {
        task->setContext(this);

        for(int32_t i = 0; i < task->outputCount(); i++) {
            Texture *texture = task->output(i);
            if(texture) {
                addTextureBuffer(texture);
            }
        }

        if(before) {
            auto it = std::find(m_renderTasks.begin(), m_renderTasks.end(), before);
            m_renderTasks.insert(it, task);
        } else {
            m_renderTasks.push_back(task);
        }
    }
}
/*!
    Returns the list of rendering tasks associated with the pipeline context.
*/
const std::list<PipelineTask *> &PipelineContext::renderTasks() const {
    return m_renderTasks;
}
/*!
    Returns a list of names of the global textures.
*/
std::list<std::string> PipelineContext::renderTextures() const {
    std::list<std::string> result;
    for(auto &it : m_textureBuffers) {
        result.push_back(it.first);
    }

    return result;
}
/*!
    Draws the specified \a list of Renderable compoenents on the given \a layer and \a flags.
*/
void PipelineContext::drawRenderers(const RenderList &list, uint32_t layer, uint32_t flags) {
    uint32_t lastHash = 0;
    uint32_t lastSub = 0;
    Mesh *lastMesh = nullptr;
    MaterialInstance *lastInstance = nullptr;

    for(auto it : list) {
        if(it) {
            Actor *actor = it->actor();

            if((flags == 0 || actor->flags() & flags) && actor->layers() & layer) {
                for(int32_t i = 0; i < it->m_materials.size(); i++) {
                    MaterialInstance *instance = it->m_materials[i];
                    if(instance->transform() == nullptr) {
                        instance->setTransform(it->transform());
                    }

                    uint32_t hash = it->instanceHash(i);
                    if(lastHash != hash || (lastInstance != nullptr && lastInstance->material() != instance->material())) {
                        if(lastInstance != nullptr) {
                            m_buffer->drawMesh(lastMesh, lastSub, layer, *lastInstance);
                            lastInstance->resetBatches();
                        }

                        lastHash = hash;
                        lastMesh = it->meshToDraw();
                        lastInstance = instance;
                        lastSub = i;
                    } else if(lastInstance != nullptr) {
                        lastInstance->batch(*instance);
                    }
                }
            }
        }
    }

    // do the last call
    if(lastInstance != nullptr) {
        m_buffer->drawMesh(lastMesh, lastSub, layer, *lastInstance);
        lastInstance->resetBatches();
    }
}
/*!
    Returns the list of scene components relevant for rendering.
*/
RenderList &PipelineContext::sceneComponents() {
    return m_sceneComponents;
}
/*!
    Returns the list of culled scene components based on frustum culling.
*/
RenderList &PipelineContext::culledComponents() {
    return m_frustumCulling ? m_culledComponents : m_sceneComponents;
}
/*!
    Returns the list of scene lights relevant for rendering.
*/
std::list<BaseLight *> &PipelineContext::sceneLights() {
    return m_sceneLights;
}
/*!
    Returns the list of filtered scene post effect settings relevant for rendering.
*/
std::list<std::pair<const PostProcessSettings *, float>> &PipelineContext::culledPostEffectSettings() {
    return m_culledPostProcessSettings;
}
/*!
    Returns the currently set camera for rendering.
*/
Camera *PipelineContext::currentCamera() const {
    return m_camera;
}
/*!
    Filters out an incoming \a list which are not in the \a frustum.
    Returns filtered list. The output parameter returns a bounding \a box for filtered objects.
*/
void PipelineContext::frustumCulling(const Frustum &frustum, const RenderList &in, RenderList &out, AABBox *box) {
    if(box) {
        box->extent = Vector3(-1.0f);
    }

    out.clear();
    for(auto it : in) {
        AABBox bb = it->bound();
        if(bb.extent.x < 0.0f || frustum.contains(bb)) {
            out.push_back(it);
            if(box) {
                box->encapsulate(bb);
            }
        }
    }
}
/*!
    Returns the bounding box representing the world-bound.
*/
AABBox PipelineContext::worldBound() const {
    return m_worldBound;
}
