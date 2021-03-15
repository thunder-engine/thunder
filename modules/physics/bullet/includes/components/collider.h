#ifndef COLLIDER_H
#define COLLIDER_H

#include "components/component.h"

#include <btBulletDynamicsCommon.h>

class Collider : public Component {
    A_REGISTER(Collider, Component, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Collider();
    ~Collider() override;

    virtual void update();

    virtual btCollisionShape *shape();

    btDynamicsWorld *world() const;
    void setWorld(btDynamicsWorld *world);

protected:
    virtual void createCollider();

protected:
    btCollisionShape *m_pCollisionShape;

    btCollisionObject *m_pCollisionObject;

    btDynamicsWorld *m_pWorld;

};

#endif // COLLIDER_H
