#ifndef RENDERRTSYSTEM_H
#define RENDERRTSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

class Engine;
class MeshRender;
class Material;

struct PhotonData {
    Vector3 rgb;
    Vector3 pos;
    Vector3 dir;
    float distance;
};
typedef list<PhotonData> PhotonList;

typedef list<uint32_t> IndexList;

struct PhotonNodeData {
    PhotonList i;
    AABBox aabb;
    PhotonNodeData *left;
    PhotonNodeData *right;
};

struct TriangleData {
    MeshRender *instance;

    uint32_t v[3];
};
typedef list<TriangleData> TriangleList;
typedef vector<TriangleData> TriangleVector;

struct BvhNodeData {
    Vector3 min;
    Vector3 max;

    uint32_t offset;
    uint32_t count;

    int left;
    int escape;
};
typedef vector<BvhNodeData> BvhVector;

class RenderRTSystem : public RenderSystem {
public:
    enum {
        BASIC = 0,
        BAKING
    };

public:
    RenderRTSystem(Engine *engine);
    ~RenderRTSystem() override;

    const BvhVector &bvh() const;

private:
    bool init() override;
    const char *name() const override;
    void update(Scene *scene) override;
    int threadPolicy() const override;

    void combineComponents(Object *object);

    void composeBVH(int mode);
    void insertMesh(MeshRender *mesh);

    void buildTriangleTree(BvhNodeData *node, TriangleList &list, int &index, int depth, bool uv = false);
    inline AABBox calcBox(TriangleList &list, bool uv);

    void emitPhotons();
    void buildPhotonsTree(PhotonNodeData *node, int depth);

private:
    ObjectList m_Components;

    PhotonNodeData m_PhotonTree;
    PhotonList m_Photons;

    TriangleVector m_Triangles;
    BvhNodeData m_TriangleTree;
    BvhNodeData m_UVTree;

    Vector3Vector m_Vertices;
    Vector3Vector m_Normals;
    Vector2Vector m_Uvs;

    BvhVector m_BVH;

    int m_DepthIndex;

    Engine *m_pEngine;

    bool m_BvhDirty;
};

#endif // RENDERRTSYSTEM_H
