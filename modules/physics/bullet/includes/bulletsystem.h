#ifndef BULLETSYSTEM_H
#define BULLETSYSTEM_H

#include <system.h>

class Engine;
class Collider;
class Joint;

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDynamicsWorld;

class BulletSystem : public System {
public:
    BulletSystem(Engine *engine);
    ~BulletSystem() override;

private:
    bool init() override { return true; }

    void update(World *world) override;

    int threadPolicy() const override;

    void addObject(Object *object) override;

    void removeObject(Object *object) override;

    static bool rayCast(System *system, World *world, const Ray &ray, float distance, Ray::Hit *hit);

protected:
    unordered_map<uint32_t, btDynamicsWorld *> m_worlds;

    list<Collider *> m_colliderList;

    list<Joint *> m_jointList;

    btDefaultCollisionConfiguration *m_collisionConfiguration;

    btCollisionDispatcher *m_dispatcher;

    btBroadphaseInterface *m_overlappingPairCache;

    btSequentialImpulseConstraintSolver *m_solver;

};

#endif // BULLETSYSTEM_H
