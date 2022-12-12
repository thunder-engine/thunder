#ifndef SKINNEDMESHRENDER_H
#define SKINNEDMESHRENDER_H

#include "renderable.h"

class Armature;
class Mesh;
class Material;
class MaterialInstance;

class ENGINE_EXPORT SkinnedMeshRender : public Renderable {
    A_REGISTER(SkinnedMeshRender, Renderable, Components/3D);

    A_PROPERTIES(
        A_PROPERTYEX(Armature *, armature, SkinnedMeshRender::armature, SkinnedMeshRender::setArmature, "editor=Component"),
        A_PROPERTYEX(MeshGroup *, mesh, SkinnedMeshRender::mesh, SkinnedMeshRender::setMesh, "editor=Asset"),
        A_PROPERTYEX(Material *, material, SkinnedMeshRender::material, SkinnedMeshRender::setMaterial, "editor=Asset")
    )
    A_NOMETHODS()

public:
    SkinnedMeshRender();

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

    Armature *armature() const;
    void setArmature(Armature *armature);

private:
    AABBox localBound() const override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void onReferenceDestroyed() override;
#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif
private:
    Mesh *m_mesh;

    MaterialInstance *m_material;

    Armature *m_armature;

};

#endif // SKINNEDMESHRENDER_H
