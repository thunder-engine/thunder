#ifndef COLLIDER_H
#define COLLIDER_H

#include "components/component.h"

#include <btBulletDynamicsCommon.h>

class Collider : public Component {
    A_REGISTER(Collider, Component, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(entered),
        A_SIGNAL(stay),
        A_SIGNAL(exited)
    )

public:
    Collider();
    ~Collider() override;

    virtual void update();

    virtual btCollisionShape *shape();

    btDynamicsWorld *world() const;
    void setWorld(btDynamicsWorld *world);

    void destroyShape();

    void entered();
    void stay();
    void exited();

protected:
    virtual void createCollider();

    void dirtyContacts();

    void cleanContacts();

    void setContact(Collider *other);

protected:
    friend class BulletSystem;

    typedef unordered_map<uint32_t, bool> CollisionMap;

    CollisionMap m_Collisions;

    btCollisionShape *m_pCollisionShape;

    btCollisionObject *m_pCollisionObject;

    btDynamicsWorld *m_pWorld;

};

#endif // COLLIDER_H
