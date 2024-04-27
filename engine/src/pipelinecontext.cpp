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

#define OVERRIDE "texture0"
#define RADIANCE_MAP "radianceMap"

/*!
    \class PipelineContext
    \brief Class responsible for managing the rendering pipeline context.
    \inmodule Engine

    PipelineContext is a class responsible for managing the rendering pipeline context, including rendering tasks, camera settings, and post-processing effects.
*/

PipelineContext::PipelineContext() :
        m_pipeline(nullptr),
        m_buffer(Engine::objectCreate<CommandBuffer>()),
        m_postProcessSettings(new PostProcessSettings),
        m_finalMaterial(nullptr),
        m_defaultTarget(Engine::objectCreate<RenderTarget>()),
        m_radianceMap(Engine::objectCreate<Texture>(RADIANCE_MAP)),
        m_camera(nullptr),
        m_width(64),
        m_height(64),
        m_frustumCulling(true) {

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultPostEffect.shader");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
    }

    setPipeline(Engine::loadResource<Pipeline>(Engine::value(".pipeline", ".embedded/Deferred.pipeline").toString()));

    m_radianceMap->setFormat(Texture::RGBA8);
    m_radianceMap->resize(2, 2);
    auto &surface = m_radianceMap->surface(0);

    uint32_t v = 0x00000000;//0x00352400;
    uint32_t *dst = reinterpret_cast<uint32_t *>(surface[0].data());
    for(uint8_t i = 0; i < 4; i++) {
        *dst = v;
        dst++;
    }

    m_buffer->setGlobalTexture(m_radianceMap->name().c_str(), m_radianceMap);
}

PipelineContext::~PipelineContext() {
    m_textureBuffers.clear();
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
    setCurrentCamera(camera);

    for(auto it : m_renderTasks) {
        if(it->isEnabled()) {
            it->exec(*this);
        }
    }

    m_finalMaterial->setTexture(OVERRIDE, m_renderTasks.back()->output(0));

    // Finish
    m_buffer->setRenderTarget(m_defaultTarget);
    m_buffer->clearRenderTarget();
    m_buffer->drawMesh(Matrix4(), defaultPlane(), 0, CommandBuffer::UI, *m_finalMaterial);
}
/*!
    Sets the current \a camera and updates associated matrices in the command buffer.
*/
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
/*!
     Resets the camera view and projection matrices in the command buffer.
*/
void PipelineContext::cameraReset() {
    m_buffer->setViewProjection(m_cameraView, m_cameraProjection);
}
/*!
    \internal
    Sets the maximum texture \a size in the command buffer for shadow mapping.
*/
void PipelineContext::setMaxTexture(uint32_t size) {
    m_buffer->setGlobalValue("shadow.pageSize", Vector4(1.0f / size, 1.0f / size, size, size));
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
    \internal
    Analyzes the scene graph \a world to determine which components and lights are relevant for rendering. Updates post-process settings.
*/
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

    for(auto &it : m_renderTasks) {
        it->analyze(world);
        it->setSettings(*m_postProcessSettings);
    }
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
Texture *PipelineContext::textureBuffer(const string &name) {
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
            string taskName = m_pipeline->renderTaskName(i);
            PipelineTask *task = dynamic_cast<PipelineTask *>(Engine::objectCreate(taskName, taskName, this));
            if(task) {
                insertRenderTask(task);
            } else {
                aError() << "Wrong Pipeline Task type:" << taskName.c_str();
            }
        }

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
}
/*!
     Inserts a rendering \a task into the pipeline context. Optionally, specifies the task to insert \a before.
*/
void PipelineContext::insertRenderTask(PipelineTask *task, PipelineTask *before) {
    for(uint32_t i = 0; i < task->outputCount(); i++) {
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
/*!
    Returns the list of rendering tasks associated with the pipeline context.
*/
const list<PipelineTask *> &PipelineContext::renderTasks() const {
    return m_renderTasks;
}
/*!
    Returns a list of names of the global textures.
*/
list<string> PipelineContext::renderTextures() const {
    list<string> result;
    for(auto &it : m_textureBuffers) {
        result.push_back(it.first);
    }

    return result;
}
/*!
    Draws the specified \a list of Renderable compoenents on the given \a layer.
*/
void PipelineContext::drawRenderers(const list<Renderable *> &list, uint32_t layer, uint32_t flags) {
    for(auto it : list) {
        if(flags == 0 || it->actor()->hideFlags() & flags) {
            it->draw(*m_buffer, layer);
        }
    }
}
/*!
    Returns the list of scene components relevant for rendering.
*/
list<Renderable *> &PipelineContext::sceneComponents() {
    return m_sceneComponents;
}
/*!
    Returns the list of culled scene components based on frustum culling.
*/
list<Renderable *> &PipelineContext::culledComponents() {
    return m_frustumCulling ? m_culledComponents : m_sceneComponents;
}
/*!
    Returns the list of scene lights relevant for rendering.
*/
list<BaseLight *> &PipelineContext::sceneLights() {
    return m_sceneLights;
}
/*!
    Returns the currently set camera for rendering.
*/
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
/*!
    Returns the bounding box representing the world-bound.
*/
AABBox PipelineContext::worldBound() const {
    return m_worldBound;
}
