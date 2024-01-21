#include "bulletsystem.h"

#include <assert.h>

#include <cstring>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

#include <log.h>
#include <timer.h>

#include <components/world.h>
#include <components/actor.h>
#include <components/scene.h>

#include <systems/resourcesystem.h>

#include "components/rigidbody.h"
#include "components/collider.h"
#include "components/boxcollider.h"
#include "components/spherecollider.h"
#include "components/capsulecollider.h"
#include "components/meshcollider.h"
#include "components/charactercontroller.h"

#include "components/joint.h"
#include "components/springjoint.h"
#include "components/hingejoint.h"
#include "components/fixedjoint.h"

#include "resources/physicmaterial.h"

#include "bulletdebug.h"

BulletSystem::BulletSystem(Engine *engine) :
        System(),
        m_collisionConfiguration(new btDefaultCollisionConfiguration),
        m_dispatcher(new btCollisionDispatcher(m_collisionConfiguration)),
        m_overlappingPairCache(new btDbvtBroadphase),
        m_solver(new btSequentialImpulseConstraintSolver) {

    PROFILE_FUNCTION();

    Collider::registerClassFactory(this);

    RigidBody::registerClassFactory(this);
    VolumeCollider::registerClassFactory(this);

    BoxCollider::registerClassFactory(this);
    SphereCollider::registerClassFactory(this);
    CapsuleCollider::registerClassFactory(this);

    CharacterController::registerClassFactory(this);
    MeshCollider::registerClassFactory(this);

    Joint::registerClassFactory(this);
    SpringJoint::registerClassFactory(this);
    HingeJoint::registerClassFactory(this);
    FixedJoint::registerClassFactory(this);

    PhysicMaterial::registerClassFactory(engine->resourceSystem());

    m_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}

BulletSystem::~BulletSystem() {
    PROFILE_FUNCTION();

    {
        auto it = m_colliderList.begin();
        while(it != m_colliderList.end()) {
            (*it)->setBulletWorld(nullptr);

            delete *it;
            it = m_colliderList.begin();
        }
    }

    {
        auto it = m_jointList.begin();
        while(it != m_jointList.end()) {
            (*it)->setBulletWorld(nullptr);

            delete *it;
            it = m_jointList.begin();
        }
    }

    for(auto &it : m_worlds) {
        delete it.second;
    }

    delete m_solver;
    delete m_overlappingPairCache;
    delete m_dispatcher;
    delete m_collisionConfiguration;

    Collider::unregisterClassFactory(this);

    RigidBody::unregisterClassFactory(this);
    VolumeCollider::unregisterClassFactory(this);

    BoxCollider::unregisterClassFactory(this);
    SphereCollider::unregisterClassFactory(this);
    CapsuleCollider::unregisterClassFactory(this);

    MeshCollider::unregisterClassFactory(this);
    CharacterController::unregisterClassFactory(this);

    Joint::unregisterClassFactory(this);
    SpringJoint::unregisterClassFactory(this);
    HingeJoint::unregisterClassFactory(this);
    FixedJoint::registerClassFactory(this);

    setName("Bullet Physics");
}

void BulletSystem::update(World *world) {
    PROFILE_FUNCTION();

    if(Engine::isGameMode()) {
        btDynamicsWorld *dynamicWorld = nullptr;
        auto it = m_worlds.find(world->uuid());
        if(it == m_worlds.end()) {
            dynamicWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
#ifdef SHARED_DEFINE
            BulletDebug *dbg = new BulletDebug;
            dbg->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawConstraints | btIDebugDraw::DBG_DrawConstraintLimits);
            dynamicWorld->setDebugDrawer(dbg);
#endif
            m_worlds[world->uuid()] = dynamicWorld;
            world->setRayCastHandler(&rayCast, this);
        } else {
            dynamicWorld = it->second;
        }

        for(auto &it : m_colliderList) {
            it->dirtyContacts();
        }

        for(int i = 0; i < m_dispatcher->getNumManifolds(); i++) {
            btPersistentManifold *contact = m_dispatcher->getManifoldByIndexInternal(i);

            const btCollisionObject *a = static_cast<const btCollisionObject*>(contact->getBody0());
            const btCollisionObject *b = static_cast<const btCollisionObject*>(contact->getBody1());

            Collider *colliderA = reinterpret_cast<Collider *>(a->getUserPointer());
            Collider *colliderB = reinterpret_cast<Collider *>(b->getUserPointer());

            if(colliderA && colliderB) {
                colliderA->setContact(colliderB);
                colliderB->setContact(colliderA);
            }
        }

        for(auto &it : m_colliderList) {
            if(it->m_world == nullptr && it->world() == world) {
                it->setBulletWorld(dynamicWorld);
            }

            it->update();
            it->cleanContacts();
        }

        for(auto &it : m_jointList) {
            if(it->m_world == nullptr && it->world() == world) {
                it->setBulletWorld(dynamicWorld);
            }
        }

        dynamicWorld->stepSimulation(Timer::deltaTime(), 4);
    }
}

int BulletSystem::threadPolicy() const {
    return Pool;
}

void BulletSystem::addObject(Object *object) {
    Collider *collider = dynamic_cast<Collider *>(object);
    if(collider) {
        m_colliderList.push_back(collider);
    } else {
        Joint *joint = dynamic_cast<Joint *>(object);
        if(joint) {
            m_jointList.push_back(joint);
        } else {
            System::addObject(object);
        }
    }
}

void BulletSystem::removeObject(Object *object) {
    m_colliderList.remove(static_cast<Collider *>(object));
    m_jointList.remove(static_cast<Joint *>(object));

    System::removeObject(object);
}

bool BulletSystem::rayCast(System *system, World *world, const Ray &ray, float distance, Ray::Hit *hit) {
    BulletSystem *bullet = static_cast<BulletSystem *>(system);
    auto it = bullet->m_worlds.find(world->uuid());
    if(it != bullet->m_worlds.end()) {
        btVector3 from(ray.pos.x, ray.pos.y, ray.pos.z);
        btVector3 to(ray.pos.x + ray.dir.x * distance,
                     ray.pos.y + ray.dir.y * distance,
                     ray.pos.z + ray.dir.z * distance);

        btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
        closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;

        it->second->rayTest(from, to, closestResults);
        if(closestResults.hasHit()) {
             if(hit) {
                hit->object = reinterpret_cast<Object *>(closestResults.m_collisionObject->getUserPointer());

                hit->distance = closestResults.m_closestHitFraction;

                hit->normal = Vector3(closestResults.m_hitNormalWorld.x(),
                                      closestResults.m_hitNormalWorld.y(),
                                      closestResults.m_hitNormalWorld.z());

                hit->point = Vector3 (closestResults.m_hitPointWorld.x(),
                                      closestResults.m_hitPointWorld.y(),
                                      closestResults.m_hitPointWorld.z());
            }
            return true;
        }
    }
    return false;
}
