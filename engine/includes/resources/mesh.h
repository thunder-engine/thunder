#ifndef MESH_H
#define MESH_H

#include <amath.h>

#include "resource.h"

class Material;
class MeshPrivate;

typedef vector<uint32_t> IndexVector;

class NEXT_LIBRARY_EXPORT Lod {
    A_PROPERTIES(
        A_PROPERTY(Material *, material, Lod::material, Lod::setMaterial)
    )
    A_NOMETHODS()

public:
    Lod();

    bool operator== (const Lod &right) const;

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

private:
    friend class Mesh;

private:
    Vector4Vector m_Colors;

    Vector4Vector m_Weights;

    Vector4Vector m_Bones;

    Vector3Vector m_Normals;

    Vector3Vector m_Tangents;

    Vector3Vector m_Vertices;

    Vector2Vector m_Uv0;

    Vector2Vector m_Uv1;

    IndexVector m_Indices;

    Material *m_Material;
};
typedef deque<Lod> LodQueue;

class NEXT_LIBRARY_EXPORT Mesh : public Resource {
    A_REGISTER(Mesh, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(AABBox, bound, Mesh::bound, Mesh::setBound),
        A_PROPERTY(int, mode, Mesh::mode, Mesh::setMode),
        A_PROPERTY(int, flags, Mesh::flags, Mesh::setFlags)
    )
    A_METHODS(
        A_METHOD(void, Mesh::clear),
        A_METHOD(bool, Mesh::isDynamic),
        A_METHOD(void, Mesh::makeDynamic),
        A_METHOD(int,  Mesh::lodsCount),
        A_METHOD(void, Mesh::addLod),
        A_METHOD(Lod *, Mesh::lod),
        A_METHOD(void, Mesh::setLod)
    )
    A_ENUMS(
        A_ENUM(MeshAttributes,
               A_VALUE(Color),
               A_VALUE(Uv0),
               A_VALUE(Uv1),
               A_VALUE(Normals),
               A_VALUE(Tangents),
               A_VALUE(Skinned)),

        A_ENUM(TriangleModes,
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

    enum TriangleModes {
        Triangles = 0,
        Lines,
        TriangleStrip,
        LineStrip,
        TriangleFan
    };

public:
    Mesh ();
    ~Mesh ();

    virtual void clear();

    bool isDynamic () const;
    void makeDynamic ();

    int lodsCount() const;

    AABBox bound() const;
    void setBound(const AABBox &box);

    int mode() const;
    void setMode(int mode);

    int flags() const;
    void setFlags(int flags);

    int addLod(const Lod *lod);

    Lod *lod(int lod) const;
    void setLod(int lod, Lod *data);

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;

private:
    MeshPrivate *p_ptr;

};

#endif // MESH_H
