#ifndef POSE_H
#define POSE_H

#include <amath.h>

#include "resource.h"

class PosePrivate;

class NEXT_LIBRARY_EXPORT Bone {
    A_PROPERTIES(
        A_PROPERTY(int, index, Bone::index, Bone::setIndex),
        A_PROPERTY(Vector3, position, Bone::position, Bone::setPosition),
        A_PROPERTY(Vector3, rotation, Bone::rotation, Bone::setRotation),
        A_PROPERTY(Vector3, scale, Bone::scale, Bone::setScale)
    )
    A_NOMETHODS()

public:
    Bone();

    bool operator== (const Bone &bone) const;

    int index() const;
    void setIndex(int value);

    const Vector3 &position() const;
    void setPosition(const Vector3 &value);

    const Vector3 &rotation() const;
    void setRotation(const Vector3 &value);

    const Vector3 &scale() const;
    void setScale(const Vector3 &value);

protected:
    uint32_t m_index;
    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_scale;
};

class NEXT_LIBRARY_EXPORT Pose : public Resource {
    A_REGISTER(Pose, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(Bone *, Pose::bone),
        A_METHOD(void, Pose::addBone),
        A_METHOD(int, Pose::boneCount)
    )

public:
    Pose();
    ~Pose();

    void addBone(Bone *bone);

    const Bone *bone(int index) const;

    int boneCount() const;

    static void registerSuper(ObjectSystem *system);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    PosePrivate *p_ptr;

};

#endif // POSE_H
