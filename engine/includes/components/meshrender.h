#ifndef MESHRENDER_H
#define MESHRENDER_H

#include <renderable.h>

#include <mesh.h>

class Material;
class MaterialInstance;

class ENGINE_EXPORT MeshRender : public Renderable {
    A_REGISTER(MeshRender, Renderable, Components/3D);

    A_PROPERTIES(
        A_PROPERTYEX(Mesh *, mesh, MeshRender::mesh, MeshRender::setMesh, "editor=Asset"),
        A_PROPERTYEX(list<Material *>, materials, MeshRender::materials, MeshRender::setMaterials, "editor=Asset")
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
    void setMaterialsList(const list<Material *> &materials) override;

    AABBox localBound() const override;

    Mesh *meshToDraw() const override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void composeComponent() override;

    void setProperty(const char *name, const Variant &value) override;

protected:
    Mesh *m_mesh;

};

#endif // MESHRENDER_H
