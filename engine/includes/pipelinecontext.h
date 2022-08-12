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
class PostProcessor;

class PostProcessVolume;

class AtlasNode;

class Renderable;

class ENGINE_EXPORT PipelineContext : public Object {
    A_REGISTER(PipelineContext, Object, System)

public:
    PipelineContext();
    ~PipelineContext();

    virtual void draw(Camera &camera);

    virtual void drawUi(Camera &camera);

    virtual void finish();

    virtual void resize(int32_t width, int32_t height);

    virtual void analizeScene(SceneGraph *graph, RenderSystem *render);

    void cameraReset(Camera &camera);

    void setRenderTarget(const string &name);

    Texture *renderTexture(const string &name) const;
    void setRenderTexture(const string &name, Texture *texture);

    RenderTarget *defaultTarget();
    void setDefaultTarget(RenderTarget *target);

    Texture *debugTexture() const;
    void setDebugTexture(const string &string);

    CommandBuffer *buffer() const;

    const list<PostProcessor *> &postEffects() const;

    list<string> renderTextures() const;

    RenderTarget *requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count);

protected:
    void drawComponents(uint32_t layer, list<Renderable *> &list);

    void postProcess(RenderTarget *source, uint32_t layer);

    void sortRenderables(list<Renderable *> &in, const Vector3 &origin);

    void cleanShadowCache();
    void updateShadows(Camera &camera);

    void combineComponents(Object *object, bool update);

protected:
    typedef map<string, Texture *> BuffersMap;
    typedef map<string, RenderTarget *> TargetsMap;

    list<Renderable *> m_sceneComponents;
    list<Renderable *> m_sceneLights;
    list<Renderable *> m_uiComponents;
    list<Renderable *> m_filter;

    list<PostProcessVolume *> m_postProcessVolume;

    BuffersMap m_textureBuffers;
    TargetsMap m_renderTargets;

    list<PostProcessor *> m_postEffects;

    unordered_map<uint32_t, pair<RenderTarget *, vector<AtlasNode *>>> m_tiles;
    unordered_map<RenderTarget *, AtlasNode *> m_shadowPages;

    CommandBuffer *m_buffer;

    Mesh *m_plane;
    MaterialInstance *m_finalMaterial;
    MaterialInstance *m_effectMaterial;

    RenderTarget *m_defaultTarget;

    Texture *m_final;
    Texture *m_debugTexture;

    RenderSystem *m_system;

    int32_t m_width;
    int32_t m_height;

};

#endif // PIPELINE
