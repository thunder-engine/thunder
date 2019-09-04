#ifndef MESHRENDER_H
#define MESHRENDER_H

#include "renderable.h"

class Material;
class Mesh;
class MeshRenderPrivate;

class NEXT_LIBRARY_EXPORT MeshRender : public Renderable {
    A_REGISTER(MeshRender, Renderable, Components);

    A_PROPERTIES (
        A_PROPERTYEX(Mesh *, Mesh, MeshRender::mesh, MeshRender::setMesh, "editor=Template"),
        A_PROPERTYEX(Material *, Material, MeshRender::material, MeshRender::setMaterial, "editor=Template")
    )
    A_NOMETHODS()

public:
    MeshRender();
    ~MeshRender();

    Mesh *mesh () const;
    void setMesh (Mesh *mesh);

    Material *material () const;
    void setMaterial (Material *material);

    AABBox bound () const override;

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

private:
    MeshRenderPrivate *p_ptr;

};

#endif // MESHRENDER_H
