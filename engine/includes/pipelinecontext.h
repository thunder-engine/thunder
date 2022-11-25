#ifndef PIPELINECONTEXT
#define PIPELINECONTEXT

#include <cstdint>
#include <unordered_map>

#include <amath.h>

#include "resource.h"

class RenderSystem;
class CommandBuffer;

class Scene;
class Camera;

class Mesh;
class MaterialInstance;
class Texture;
class RenderTarget;
class PipelinePass;

class PostProcessVolume;

class AtlasNode;

class Renderable;

class ShadowMap;

class ENGINE_EXPORT PipelineContext : public Object {
    A_REGISTER(PipelineContext, Object, System)

public:
    PipelineContext();
    ~PipelineContext();

    CommandBuffer *buffer() const;

    void analizeScene(SceneGraph *graph);

    void draw(Camera *camera);

    void cameraReset();

    void drawRenderers(uint32_t layer, const list<Renderable *> &list);

    void setMaxTexture(uint32_t size);

    RenderTarget *defaultTarget();
    void setDefaultTarget(RenderTarget *target);

    Texture *textureBuffer(const string &string);

    Texture *debugTexture() const;
    void setDebugTexture(const string &string);

    void addRenderPass(PipelinePass *pass);

    const list<PipelinePass *> &renderPasses() const;

    list<string> renderTextures() const;

    list<Renderable *> &sceneComponents();
    list<Renderable *> &sceneLights();
    list<Renderable *> &culledComponents();
    list<Renderable *> &uiComponents();

    void setCurrentCamera(Camera *camera);
    Camera *currentCamera() const;

    void showUiAsSceneView();

    void resize(int32_t width, int32_t height);

    // Shadow map management functions

    RenderTarget *requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count);

    static Mesh *defaultPlane();

protected:
    void sortRenderables(list<Renderable *> &in, const Vector3 &origin);

    void combineComponents(Object *object, bool update);

protected:
    typedef map<string, Texture *> BuffersMap;
    typedef map<string, RenderTarget *> TargetsMap;

    Matrix4 m_cameraView;
    Matrix4 m_cameraProjection;

    list<Renderable *> m_sceneComponents;
    list<Renderable *> m_sceneLights;
    list<Renderable *> m_uiComponents;
    list<Renderable *> m_culledComponents;

    list<PostProcessVolume *> m_postProcessVolume;

    BuffersMap m_textureBuffers;

    list<PipelinePass *> m_renderPasses;

    CommandBuffer *m_buffer;

    MaterialInstance *m_finalMaterial;

    RenderTarget *m_defaultTarget;

    Texture *m_final;
    Texture *m_debugTexture;

    Camera *m_camera;

    int32_t m_width;
    int32_t m_height;

    bool m_uiAsSceneView;

};

#endif // PIPELINECONTEXT
