#ifndef MESHRENDER_H
#define MESHRENDER_H

#include "renderable.h"

class Mesh;
class Material;
class MaterialInstance;

class ENGINE_EXPORT MeshRender : public Renderable {
    A_REGISTER(MeshRender, Renderable, Components/3D);

    A_PROPERTIES(
        A_PROPERTYEX(Mesh *, mesh, MeshRender::mesh, MeshRender::setMesh, "editor=Asset"),
        A_PROPERTYEX(Material *, material, MeshRender::material, MeshRender::setMaterial, "editor=Asset")
    )
    A_NOMETHODS()

public:
    MeshRender();

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

private:
    AABBox bound() const override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void composeComponent() override;

private:
    Mesh *m_mesh;

    MaterialInstance *m_material;

};

#endif // MESHRENDER_H
