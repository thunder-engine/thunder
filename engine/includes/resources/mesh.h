#ifndef MESH_H
#define MESH_H

#include <amath.h>

#include "resource.h"

class Material;

typedef vector<uint32_t> IndexVector;

class ENGINE_EXPORT Mesh : public Resource {
    A_REGISTER(Mesh, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(Material *, material, Mesh::material, Mesh::setMaterial)
    )
    A_METHODS(
        A_METHOD(bool, Mesh::isDynamic),
        A_METHOD(void, Mesh::makeDynamic)
    )
    A_ENUMS(
        A_ENUM(MeshAttributes,
               A_VALUE(Color),
               A_VALUE(Uv0),
               A_VALUE(Uv1),
               A_VALUE(Normals),
               A_VALUE(Tangents),
               A_VALUE(Skinned)),

        A_ENUM(Topology,
               A_VALUE(Triangles),
               A_VALUE(Lines),
               A_VALUE(TriangleStrip),
               A_VALUE(LineStrip),
               A_VALUE(TriangleFan))
    )

public:
    enum MeshAttributes {
        Color    = (1<<0),
        Uv0      = (1<<1),
        Uv1      = (1<<2),
        Normals  = (1<<3),
        Tangents = (1<<4),
        Skinned  = (1<<5),
    };

    enum TriangleTopology {
        Triangles = 0,
        Lines,
        TriangleStrip,
        LineStrip,
        TriangleFan
    };

public:
    Mesh();

    bool operator== (const Mesh &right) const;

    void operator= (const Mesh &right);

    bool isDynamic() const;
    void makeDynamic();

    Material *material() const;
    void setMaterial(Material *material);

    IndexVector &indices();
    void setIndices(const IndexVector &indices);

    Vector4Vector &colors();
    void setColors(const Vector4Vector &colors);

    Vector4Vector &weights();
    void setWeights(const Vector4Vector &weights);

    Vector4Vector &bones();
    void setBones(const Vector4Vector &bones);

    Vector3Vector &vertices();
    void setVertices(const Vector3Vector &vertices);

    Vector3Vector &normals();
    void setNormals(const Vector3Vector &normals);

    Vector3Vector &tangents();
    void setTangents(const Vector3Vector &tangents);

    Vector2Vector &uv0();
    void setUv0(const Vector2Vector &uv0);

    Vector2Vector &uv1();
    void setUv1(const Vector2Vector &uv1);

    int topology() const;
    void setTopology(int mode);

    int flags() const;
    void setFlags(int flags);

    AABBox bound() const;
    void setBound(const AABBox &box);

    void recalcNormals();

    void recalcBounds();

    void batchMesh(Mesh &mesh, Matrix4 *transform = nullptr);

protected:
    void switchState(ResourceState state) override;
    bool isUnloadable() override;

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    AABBox m_box;

    Vector4Vector m_colors;

    Vector4Vector m_weights;

    Vector4Vector m_bones;

    Vector3Vector m_normals;

    Vector3Vector m_tangents;

    Vector3Vector m_vertices;

    Vector2Vector m_uv0;

    Vector2Vector m_uv1;

    IndexVector m_indices;

    Material *m_material;

    int m_flags;

    int m_topology;

    bool m_dynamic;

};
typedef deque<Mesh *> LodQueue;

class ENGINE_EXPORT MeshGroup : public Resource {
    A_REGISTER(MeshGroup, Resource, Resources)

    A_METHODS(
        A_METHOD(int,  MeshGroup::lodsCount),
        A_METHOD(void, MeshGroup::addLod),
        A_METHOD(Mesh *, MeshGroup::lod),
        A_METHOD(void, MeshGroup::setLod)
    )

public:
    MeshGroup();
    ~MeshGroup();

    int lodsCount() const;

    int addLod(Mesh *lod);

    Mesh *lod(int lod);
    void setLod(int lod, Mesh *data);

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    mutable LodQueue m_lods;

};

#endif // MESH_H
