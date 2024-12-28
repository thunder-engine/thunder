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

class ENGINE_EXPORT PipelineContext : public Object {
    A_REGISTER(PipelineContext, Object, System)

public:
    PipelineContext();
    ~PipelineContext();

    CommandBuffer *buffer() const;

    void draw(Camera *camera);

    void cameraReset();

    void drawRenderers(const std::list<Renderable *> &list, uint32_t layer, uint32_t flags = 0);

    void setMaxTexture(uint32_t size);

    World *world();
    void setWorld(World *world);

    RenderTarget *defaultTarget();
    void setDefaultTarget(RenderTarget *target);

    void addTextureBuffer(Texture *texture);
    Texture *textureBuffer(const std::string &name);

    std::list<std::string> renderTextures() const;

    std::list<Renderable *> &sceneComponents();
    std::list<Renderable *> &culledComponents();
    std::list<BaseLight *> &sceneLights();

    std::list<Renderable *> frustumCulling(const std::array<Vector3, 8> &frustum, std::list<Renderable *> &list, AABBox &box);

    void setPipeline(Pipeline *pipeline);
    void insertRenderTask(PipelineTask *task, PipelineTask *before = nullptr);

    const std::list<PipelineTask *> &renderTasks() const;

    AABBox worldBound() const;

    void setCurrentCamera(Camera *camera);
    Camera *currentCamera() const;

    void resize(int32_t width, int32_t height);

    static Mesh *defaultPlane();
    static Mesh *defaultCube();

private:
    void analizeGraph();

protected:
    typedef std::map<std::string, Texture *> BuffersMap;
    typedef std::map<std::string, RenderTarget *> TargetsMap;

    Matrix4 m_cameraView;
    Matrix4 m_cameraProjection;

    AABBox m_worldBound;

    std::list<Renderable *> m_sceneComponents;
    std::list<Renderable *> m_culledComponents;
    std::list<BaseLight *> m_sceneLights;

    BuffersMap m_textureBuffers;

    std::list<PipelineTask *> m_renderTasks;

    World *m_world;

    Pipeline *m_pipeline;

    CommandBuffer *m_buffer;

    PostProcessSettings *m_postProcessSettings;

    MaterialInstance *m_finalMaterial;

    RenderTarget *m_defaultTarget;

    Texture *m_radianceMap;

    Camera *m_camera;

    int32_t m_width;
    int32_t m_height;

    bool m_frustumCulling;

};

#endif // PIPELINECONTEXT
