#ifndef PIPELINE
#define PIPELINE

#include <cstdint>
#include <stack>

#include <amath.h>

#include "resource.h"

class ICommandBuffer;

class Scene;
class Camera;

class Mesh;
class MaterialInstance;
class RenderTexture;
class PostProcessor;

class PackNode;

class NEXT_LIBRARY_EXPORT Pipeline : public Resource {
    A_REGISTER(Pipeline, Resource, Resources)

public:
    Pipeline ();
    ~Pipeline ();

    virtual void draw (Scene *scene, Camera &camera);

    virtual void resize (int32_t width, int32_t height);

    void cameraReset (Camera &camera);

    RenderTexture *target (const string &target) const;

    void combineComponents (Object *object, bool first = false);

    void setTarget (uint32_t resource);

    RenderTexture *requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count);

    ICommandBuffer *buffer () const;

    static void shadowPageSize (int32_t &width, int32_t &height);
    static void setShadowPageSize (int32_t width, int32_t height);

protected:
    void drawComponents (uint32_t layer, ObjectList &list);

    void updateShadows (Camera &camera, ObjectList &list);

    RenderTexture *postProcess (RenderTexture *source);

    void sortByDistance (ObjectList &in, const Vector3 &origin);

private:
    RenderTexture *createShadowPage();

protected:
    typedef map<string, RenderTexture *> TargetMap;

    ICommandBuffer *m_Buffer;

    ObjectList m_Components;

    TargetMap m_Targets;

    Vector2 m_Screen;

    list<PostProcessor *> m_OpaqEffects;
    list<PostProcessor *> m_PostEffects;

    Mesh *m_pPlane;
    MaterialInstance *m_pSprite;

    uint32_t m_Target;

    static int32_t m_ShadowPageWidth;
    static int32_t m_ShadowPageHeight;

    unordered_map<uint32_t, pair<RenderTexture *, vector<PackNode *>>> m_Tiles;
    unordered_map<RenderTexture *, PackNode *> m_ShadowPages;
};

#endif // PIPELINE
