#ifndef JOINT_H
#define JOINT_H

#include "component.h"
#include "bullet.h"

class btTypedConstraint;
class btDynamicsWorld;

class RigidBody;

class BULLET_EXPORT Joint : public Component {
    A_REGISTER(Joint, Component, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Joint();
    ~Joint() override;

    Vector3 anchor() const;
    void setAnchor(const Vector3 anchor);

    RigidBody *connectedBody() const;

protected:
    virtual void createConstraint();

    void destroyConstraint();

    void setBulletWorld(btDynamicsWorld *world);

private:
    btTypedConstraint *m_constraint;

    btDynamicsWorld *m_world;

    RigidBody *m_rigidBodyA;
    RigidBody *m_rigidBodyB;

    Vector3 m_anchor;

    float m_breakForse;

};

#endif // JOINT_H
