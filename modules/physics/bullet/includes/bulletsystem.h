#ifndef BULLETSYSTEM_H
#define BULLETSYSTEM_H

#include <system.h>

class Engine;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDynamicsWorld;

class BulletSystem : public ISystem {
public:
    BulletSystem (Engine *engine);
    ~BulletSystem () override;

    bool init () override;

    const char *name () const override;

    void update (Scene *) override;

    bool isThreadSafe () const override;

protected:
    bool m_Inited;

    btDefaultCollisionConfiguration *m_pCollisionConfiguration;

    btCollisionDispatcher *m_pDispatcher;

    btBroadphaseInterface *m_pOverlappingPairCache;

    btSequentialImpulseConstraintSolver *m_pSolver;

    unordered_map<uint32_t, btDynamicsWorld *> m_Worlds;
};

#endif // BULLETSYSTEM_H
