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
        A_PROPERTY(Vector3, boundsCenter, SkinnedMeshRender::boundsCenter, SkinnedMeshRender::setBoundsCenter),
        A_PROPERTY(Vector3, boundsExtent, SkinnedMeshRender::boundsExtent, SkinnedMeshRender::setBoundsExtent),
        A_PROPERTYEX(MeshGroup *, mesh, SkinnedMeshRender::mesh, SkinnedMeshRender::setMesh, "editor=Asset"),
        A_PROPERTYEX(Material *, material, SkinnedMeshRender::material, SkinnedMeshRender::setMaterial, "editor=Asset"),
        A_PROPERTYEX(Armature *, armature, SkinnedMeshRender::armature, SkinnedMeshRender::setArmature, "editor=Component")
    )
    A_NOMETHODS()

public:
    SkinnedMeshRender();

    Vector3 boundsCenter() const;
    void setBoundsCenter(Vector3 center);

    Vector3 boundsExtent() const;
    void setBoundsExtent(Vector3 extent);

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Armature *armature() const;
    void setArmature(Armature *armature);

private:
    AABBox localBound() const override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void setMaterial(Material *material) override;

    void onReferenceDestroyed() override;

    void drawGizmosSelected() override;

private:
    AABBox m_bounds;

    Mesh *m_mesh;

    Armature *m_armature;

};

#endif // SKINNEDMESHRENDER_H
