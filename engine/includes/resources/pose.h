#ifndef POSE_H
#define POSE_H

#include <amath.h>

#include "resource.h"

class PosePrivate;

class NEXT_LIBRARY_EXPORT Pose : public Resource {
    A_REGISTER(Pose, Resource, Resources)

public:
    struct Bone {
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
    };

    Pose();
    ~Pose();

    void addBone (const Bone &bone);

    const Bone *bone (uint32_t index) const;

    uint32_t size () const;

private:
    void loadUserData (const VariantMap &data) override;

private:
    PosePrivate *p_ptr;

};

#endif // POSE_H
