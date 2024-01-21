#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"
#include "bullet.h"

class btCollisionShape;
class btCollisionObject;
class btDynamicsWorld;

class RigidBody;

class BULLET_EXPORT Collider : public Component {
    A_REGISTER(Collider, Component, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(Collider::entered),
        A_SIGNAL(Collider::stay),
        A_SIGNAL(Collider::exited)
    )

public:
    Collider();
    ~Collider() override;

    virtual void update();

    RigidBody *attachedRigidBody() const;
    void setAttachedRigidBody(RigidBody *body);

public: // Signals
    void entered();
    void stay();
    void exited();

protected:
    virtual void createCollider();

    virtual btCollisionShape *shape();
    btCollisionObject *collisionObject();

    btDynamicsWorld *bulletWorld() const;
    void setBulletWorld(btDynamicsWorld *world);

    void dirtyContacts();

    void cleanContacts();

    void setContact(Collider *collider);

    void destroyShape();

    void destroyCollider();

    Vector4 gizmoColor() const;

protected:
    friend class RigidBody;
    friend class Joint;
    friend class BulletSystem;

    typedef unordered_map<uint32_t, bool> CollisionMap;

    CollisionMap m_collisions;

    btCollisionShape *m_collisionShape;

    btCollisionObject *m_collisionObject;

    btDynamicsWorld *m_world;

    RigidBody *m_rigidBody;

};
typedef Collider* ColliderPtr;

#endif // COLLIDER_H
