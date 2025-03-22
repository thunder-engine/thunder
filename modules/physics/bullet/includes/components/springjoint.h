#ifndef SPRINGJOINT_H
#define SPRINGJOINT_H

#include "joint.h"

class BULLET_EXPORT SpringJoint : public Joint {
    A_OBJECT(SpringJoint, Joint, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, spring, SpringJoint::spring, SpringJoint::setSpring),
        A_PROPERTY(float, damper, SpringJoint::damper, SpringJoint::setDamper)
    )
    A_NOMETHODS()

public:
    SpringJoint();

    float damper() const;
    void setDamper(float damper);

    float spring() const;
    void setSpring(float spring);

private:
    void createConstraint() override;

    void drawGizmosSelected() override;

    void updateParams();

private:
    float m_damper;

    float m_spring;

};

#endif // SPRINGJOINT_H
