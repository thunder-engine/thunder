#ifndef SKINNEDMESHRENDER_H
#define SKINNEDMESHRENDER_H

#include <meshrender.h>

#include <armature.h>
#include <mesh.h>

class Material;
class MaterialInstance;

class ENGINE_EXPORT SkinnedMeshRender : public MeshRender {
    A_REGISTER(SkinnedMeshRender, MeshRender, Components/3D);

    A_PROPERTIES(
        A_PROPERTYEX(Armature *, armature, SkinnedMeshRender::armature, SkinnedMeshRender::setArmature, "editor=Component"),
        A_PROPERTY(Vector3, boundsCenter, SkinnedMeshRender::boundsCenter, SkinnedMeshRender::setBoundsCenter),
        A_PROPERTY(Vector3, boundsExtent, SkinnedMeshRender::boundsExtent, SkinnedMeshRender::setBoundsExtent)
    )
    A_NOMETHODS()

public:
    SkinnedMeshRender();

    Vector3 boundsCenter() const;
    void setBoundsCenter(Vector3 center);

    Vector3 boundsExtent() const;
    void setBoundsExtent(Vector3 extent);

    Armature *armature() const;
    void setArmature(Armature *armature);

private:
    void setMaterialsList(const std::list<Material *> &materials) override;

    AABBox localBound() const override;

    void setMaterial(Material *material) override;

    void onReferenceDestroyed() override;

private:
    AABBox m_bounds;

    Armature *m_armature;

};

#endif // SKINNEDMESHRENDER_H
