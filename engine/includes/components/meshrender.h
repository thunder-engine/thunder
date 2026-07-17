#ifndef MESHRENDER_H
#define MESHRENDER_H

#include <renderable.h>

#include <mesh.h>

#include <array>

class Material;
class MaterialInstance;

class ENGINE_EXPORT MeshRender : public Renderable {
    A_OBJECT(MeshRender, Renderable, Components/3D)

    A_PROPERTIES(
        A_PROPERTYEX(Mesh *, mesh, MeshRender::mesh, MeshRender::setMesh, "editor=Asset"),
        A_PROPERTYEX(Material[], materials, MeshRender::materials, MeshRender::setMaterials, "editor=Asset")
    )
    A_NOMETHODS()

public:
    MeshRender();
    ~MeshRender();

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    float blendShapeWeight(size_t index) const;
    void setBlendShapeWeight(size_t index, float weight);

    VariantList materials() const;
    void setMaterials(VariantList list);

protected:
    AABBox localBound() override;

    Mesh *meshToDraw() override;

    void drawGizmosSelected() override;

    void composeComponent() override;

    bool ensureMeshInstance();

    void updateBlendShapes();

protected:
    std::vector<float> m_blendShapeWeights;
    std::array<std::pair<Mesh *, bool>, 3> m_lods;

    TString m_meshRef;

    Mesh *m_baseMesh;
    Mesh *m_meshInstance;

};

#endif // MESHRENDER_H
