#ifndef SKINNEDMESHRENDER_H
#define SKINNEDMESHRENDER_H

#include "renderable.h"

class Armature;
class Material;
class Mesh;
class SkinnedMeshRenderPrivate;

class NEXT_LIBRARY_EXPORT SkinnedMeshRender : public Renderable {
    A_REGISTER(SkinnedMeshRender, Renderable, Components);

    A_PROPERTIES(
        A_PROPERTYEX(Armature *, armature, SkinnedMeshRender::armature, SkinnedMeshRender::setArmature, "editor=Component"),
        A_PROPERTYEX(Mesh *, mesh, SkinnedMeshRender::mesh, SkinnedMeshRender::setMesh, "editor=Template"),
        A_PROPERTYEX(Material *, material, SkinnedMeshRender::material, SkinnedMeshRender::setMaterial, "editor=Template")
    )
    A_NOMETHODS()

public:
    SkinnedMeshRender();
    ~SkinnedMeshRender() override;

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

    Armature *armature() const;
    void setArmature(Armature *armature);

private:
    AABBox bound() const override;

    void draw(ICommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif
private:
    SkinnedMeshRenderPrivate *p_ptr;

};

#endif // SKINNEDMESHRENDER_H
