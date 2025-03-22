#ifndef PIPELINECONTEXT
#define PIPELINECONTEXT

#include <cstdint>
#include <unordered_map>

#include <amath.h>

#include "resource.h"

class CommandBuffer;

class Camera;

class Mesh;
class MaterialInstance;
class Texture;
class RenderTarget;
class Pipeline;
class PipelineTask;

class BaseLight;
class Renderable;
class PostProcessSettings;
class InstancingBatch;

class Frustum;

typedef std::list<Renderable *> RenderList;

class ENGINE_EXPORT PipelineContext : public Object {
    A_OBJECT(PipelineContext, Object, System)

public:
    typedef void (*RenderCallback)(void *object);

public:
    PipelineContext();
    ~PipelineContext();

    CommandBuffer *buffer() const;

    void draw(Camera *camera);

    void cameraReset();

    void drawRenderers(const RenderList &list, uint32_t layer, uint32_t flags = 0);

    World *world();
    void setWorld(World *world);

    Texture *resultTexture();

    RenderTarget *defaultTarget();
    void setDefaultTarget(RenderTarget *target);

    void addTextureBuffer(Texture *texture);
    Texture *textureBuffer(const std::string &name);

    std::list<std::string> renderTextures() const;

    RenderList &sceneComponents();
    RenderList &culledComponents();
    std::list<BaseLight *> &sceneLights();

    std::list<std::pair<const PostProcessSettings *, float> > &culledPostEffectSettings();

    void frustumCulling(const Frustum &frustum, const RenderList &in, RenderList &out, AABBox *box = nullptr);

    void setPipeline(Pipeline *pipeline);
    void insertRenderTask(PipelineTask *task, PipelineTask *before = nullptr);

    const std::list<PipelineTask *> &renderTasks() const;

    AABBox worldBound() const;

    void setCurrentCamera(Camera *camera);
    Camera *currentCamera() const;

    void resize(int32_t width, int32_t height);

    void invalidateTasks();

    void subscribePost(RenderCallback callback, void *object);
    void unsubscribePost(void *object);

    static Mesh *defaultPlane();
    static Mesh *defaultCube();

private:
    void analizeGraph();

protected:
    typedef std::map<std::string, Texture *> BuffersMap;
    typedef std::map<std::string, RenderTarget *> TargetsMap;

    typedef std::list<std::pair<PipelineContext::RenderCallback, void *>> Callbacks;

    Callbacks m_postObservers;

    Matrix4 m_cameraView;
    Matrix4 m_cameraProjection;

    AABBox m_worldBound;

    RenderList m_sceneComponents;
    RenderList m_culledComponents;

    std::list<BaseLight *> m_sceneLights;

    std::list<std::pair<const PostProcessSettings *, float>> m_culledPostProcessSettings;

    BuffersMap m_textureBuffers;

    std::list<PipelineTask *> m_renderTasks;

    World *m_world;

    Pipeline *m_pipeline;

    CommandBuffer *m_buffer;

    MaterialInstance *m_finalMaterial;

    RenderTarget *m_defaultTarget;

    Camera *m_camera;

    int32_t m_width;
    int32_t m_height;

    bool m_frustumCulling;

    bool m_renderablesSorting;

};

#endif // PIPELINECONTEXT
