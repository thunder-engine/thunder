#ifndef SKINNEDSPRITERENDER_H
#define SKINNEDSPRITERENDER_H

#include <spriterender.h>

#include <armature.h>

class ENGINE_EXPORT SkinnedSpriteRender : public SpriteRender {
    A_OBJECT(SkinnedSpriteRender, SpriteRender, Components/2D)

    A_PROPERTIES(
        A_PROPERTYEX(Armature *, armature, SkinnedSpriteRender::armature, SkinnedSpriteRender::setArmature, "editor=Component"),
        A_PROPERTY(Vector3, boundsCenter, SkinnedSpriteRender::boundsCenter, SkinnedSpriteRender::setBoundsCenter),
        A_PROPERTY(Vector3, boundsExtent, SkinnedSpriteRender::boundsExtent, SkinnedSpriteRender::setBoundsExtent)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    SkinnedSpriteRender();

    Vector3 boundsCenter() const;
    void setBoundsCenter(const Vector3 &center);

    Vector3 boundsExtent() const;
    void setBoundsExtent(const Vector3 &extent);

    Armature *armature() const;
    void setArmature(Armature *armature);

private:
    void setMaterialsList(const std::list<Material *> &materials) override;

    AABBox localBound() override;

    void onReferenceDestroyed() override;

private:
    AABBox m_bounds;

    Armature *m_armature;

};

#endif // SKINNEDSPRITERENDER_H
