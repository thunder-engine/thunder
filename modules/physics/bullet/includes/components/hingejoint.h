#ifndef HINGEJOINT_H
#define HINGEJOINT_H

#include "joint.h"

class BULLET_EXPORT HingeJoint : public Joint {
    A_OBJECT(HingeJoint, Joint, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(Vector3, axis, HingeJoint::axis, HingeJoint::setAxis)
    )
    A_NOMETHODS()

public:
    HingeJoint();

    Vector3 axis() const;
    void setAxis(Vector3 axis);

private:
    void createConstraint() override;

    void drawGizmosSelected() override;

    void updateParams();

private:
    Vector3 m_axis;

};

#endif // HINGEJOINT_H
