#ifndef PIPELINE
#define PIPELINE

#include <cstdint>
#include <unordered_map>

#include <amath.h>

#include "resource.h"

class RenderSystem;
class ICommandBuffer;

class Scene;
class Camera;

class Mesh;
class MaterialInstance;
class RenderTexture;
class PostProcessor;

class PostProcessSettings;

class AtlasNode;

class NEXT_LIBRARY_EXPORT Pipeline : public Resource {
    A_REGISTER(Pipeline, Resource, Resources)

public:
    Pipeline();
    ~Pipeline();

    virtual void draw(Camera &camera);

    virtual void resize(int32_t width, int32_t height);

    virtual void finish();

    virtual void analizeScene(Scene *scene, RenderSystem *render);

    RenderTexture *target(const string &target) const;

    void setTarget(uint32_t resource);

    ICommandBuffer *buffer() const;

    RenderTexture *requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count);

protected:
    void cameraReset(Camera &camera);

    void drawComponents(uint32_t layer, ObjectList &list);

    void combineComponents(Object *object);

    RenderTexture *postProcess(RenderTexture *source, uint32_t layer);

    void sortByDistance(ObjectList &in, const Vector3 &origin);

    void cleanShadowCache();
    void updateShadows(Camera &camera, ObjectList &list);

protected:
    typedef map<string, RenderTexture *> TargetMap;

    ICommandBuffer *m_Buffer;

    ObjectList m_Components;
    ObjectList m_Filter;

    TargetMap m_Targets;

    Vector2 m_Screen;

    list<PostProcessor *> m_PostEffects;

    list<PostProcessSettings *> m_PostProcessSettings;

    Mesh *m_pPlane;
    MaterialInstance *m_pSprite;

    uint32_t m_Target;

    unordered_map<uint32_t, pair<RenderTexture *, vector<AtlasNode *>>> m_Tiles;
    unordered_map<RenderTexture *, AtlasNode *> m_ShadowPages;

    RenderTexture *m_pFinal;
};

#endif // PIPELINE
