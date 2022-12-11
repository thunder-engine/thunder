#include "bulletsystem.h"

#include <assert.h>

#include <cstring>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <log.h>
#include <timer.h>

#include <components/scenegraph.h>
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

    PhysicMaterial::registerClassFactory(engine->resourceSystem());

    m_overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}

BulletSystem::~BulletSystem() {
    PROFILE_FUNCTION();

    for(auto &it : m_worlds) {
        delete it.second;
    }

    for(auto &it : m_objectList) {
        static_cast<Collider *>(it)->setWorld(nullptr);
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

    setName("Bullet Physics");
}

void BulletSystem::update(SceneGraph *graph) {
    PROFILE_FUNCTION();

    if(Engine::isGameMode()) {
        btDynamicsWorld *world = nullptr;
        auto it = m_worlds.find(graph->uuid());
        if(it == m_worlds.end()) {
            world = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
#ifdef SHARED_DEFINE
            BulletDebug *dbg = new BulletDebug;
            dbg->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
            world->setDebugDrawer(dbg);
#endif
            m_worlds[graph->uuid()] = world;
        } else {
            world = it->second;
        }

        for(auto &it : m_objectList) {
            Collider *body = static_cast<Collider *>(it);
            body->dirtyContacts();
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

        for(auto &it : m_objectList) {
            Collider *body = static_cast<Collider *>(it);
            if(body->m_world == nullptr) {
                Scene *scene = body->actor()->scene();
                if(scene && scene->parent() == graph) {
                    body->setWorld(world);
                }
            }

            body->update();
            body->cleanContacts();
        }

        world->stepSimulation(Timer::deltaTime(), 4);
    }
}

int BulletSystem::threadPolicy() const {
    return Pool;
}
