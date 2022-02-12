#ifndef ARMATURE_H
#define ARMATURE_H

#include "renderable.h"

class Texture;
class Pose;
class ArmaturePrivate;

class ENGINE_EXPORT Armature : public Renderable {
    A_REGISTER(Armature, NativeBehaviour, Components/Animation);

    A_PROPERTIES(
        A_PROPERTYEX(Pose *, bindPose, Armature::bindPose, Armature::setBindPose, "editor=Template")
    )
    A_NOMETHODS()

public:
    Armature();
    ~Armature() override;

    Pose *bindPose() const;
    void setBindPose(Pose *pose);

private:
    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    Texture *texture() const;

    AABBox recalcBounds(const AABBox &aabb) const;

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;

    bool isBoneSelected(ObjectList &selected);
#endif

private:
    friend class SkinnedMeshRender;

    ArmaturePrivate *p_ptr;
};

#endif // ARMATURE_H
