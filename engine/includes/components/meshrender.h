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

    VariantList materials() const;
    void setMaterials(VariantList list);

protected:
    AABBox localBound() override;

    Mesh *meshToDraw() override;

    void drawGizmosSelected() override;

    void composeComponent() override;

protected:
    Mesh *m_mesh;

};

#endif // MESHRENDER_H
