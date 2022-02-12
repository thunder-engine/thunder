#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"
#include "bullet.h"

class btCollisionShape;
class btCollisionObject;
class btDynamicsWorld;

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

    void destroyShape();

    void entered();
    void stay();
    void exited();

protected:
    virtual void createCollider();

    virtual btCollisionShape *shape();

    btDynamicsWorld *world() const;
    void setWorld(btDynamicsWorld *world);

    void dirtyContacts();

    void cleanContacts();

    void setContact(Collider *other);

protected:
    friend class BulletSystem;
    friend class RigidBody;

    typedef unordered_map<uint32_t, bool> CollisionMap;

    CollisionMap m_Collisions;

    btCollisionShape *m_pCollisionShape;

    btCollisionObject *m_pCollisionObject;

    btDynamicsWorld *m_pWorld;

};

#endif // COLLIDER_H
