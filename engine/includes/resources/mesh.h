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
    A_NOENUMS()

public:
    Mesh();

    bool operator== (const Mesh &right) const;

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

    bool m_dynamic;

};

#endif // MESH_H
