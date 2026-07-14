#ifndef MESHRENDER_H
#define MESHRENDER_H

#include <renderable.h>

#include <mesh.h>

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
    float blendShapeWeight(const TString &name) const;
    void setBlendShapeWeight(const TString &name, float weight);
    void clearBlendShapeWeights();

    VariantList materials() const;
    void setMaterials(VariantList list);

protected:
    AABBox localBound() override;

    Mesh *meshToDraw() override;

    void drawGizmosSelected() override;

    void composeComponent() override;

    void applyBlendShapeWeights();

protected:
    Mesh *m_mesh;
    Mesh *m_meshInstance;

    std::vector<float> m_blendShapeWeights;

};

#endif // MESHRENDER_H
