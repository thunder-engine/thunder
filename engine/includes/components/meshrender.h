#ifndef MESHRENDER_H
#define MESHRENDER_H

#include "renderable.h"

class Material;
class Mesh;
class MeshRenderPrivate;

class NEXT_LIBRARY_EXPORT MeshRender : public Renderable {
    A_REGISTER(MeshRender, Renderable, Components/3D);

    A_PROPERTIES(
        A_PROPERTYEX(Mesh *, mesh, MeshRender::mesh, MeshRender::setMesh, "editor=Template"),
        A_PROPERTYEX(Material *, material, MeshRender::material, MeshRender::setMaterial, "editor=Template")
    )
    A_NOMETHODS()

public:
    MeshRender();
    ~MeshRender() override;

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

private:
    AABBox bound() const override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    MeshRenderPrivate *p_ptr;

};

#endif // MESHRENDER_H
