#ifndef JOINT_H
#define JOINT_H

#include "rigidbody.h"

class btTypedConstraint;
class btDynamicsWorld;
class btRigidBody;

class BULLET_EXPORT Joint : public Component {
    A_OBJECT(Joint, Component, General)

    A_PROPERTIES(
        A_PROPERTY(RigidBody *, connectedBody, Joint::connectedBody, Joint::setConnectedBody),
        A_PROPERTY(Vector3, anchor, Joint::anchor, Joint::setAnchor),
        A_PROPERTY(bool, autoConfigureConnectedAnchor, Joint::autoConfigureConnectedAnchor, Joint::setAutoConfigureConnectedAnchor),
        A_PROPERTY(Vector3, connectedAnchor, Joint::connectedAnchor, Joint::setConnectedAnchor)
    )
    A_NOMETHODS()

public:
    Joint();
    ~Joint() override;

    Vector3 anchor() const;
    void setAnchor(const Vector3 anchor);

    Vector3 connectedAnchor() const;
    void setConnectedAnchor(Vector3 anchor);

    RigidBody *connectedBody() const;
    void setConnectedBody(RigidBody *body);

    bool autoConfigureConnectedAnchor() const;
    void setAutoConfigureConnectedAnchor(bool anchor);

protected:
    virtual void createConstraint();

    void setBulletWorld(btDynamicsWorld *world);

    btRigidBody *getNativeBody();

    Vector4 gizmoColor() const;

    void updateAnchors();

private:
    void destroyConstraint();

    void setRigidBodyA(btRigidBody *body);

    void composeComponent() override;

protected:
    friend class RigidBody;
    friend class BulletSystem;

    btTypedConstraint *m_constraint;

    btDynamicsWorld *m_world;

    btRigidBody *m_rigidBodyB;
    RigidBody *m_rigidBodyA;

    Vector3 m_anchor;
    Vector3 m_connectedAnchor;

    float m_breakForse;

    bool m_autoConfigureConnectedAnchor;

};

#endif // JOINT_H
