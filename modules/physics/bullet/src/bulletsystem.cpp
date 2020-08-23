#include "bulletsystem.h"

#include <assert.h>

#include <cstring>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <log.h>
#include <timer.h>

#include <components/scene.h>
#include <components/actor.h>

#include "components/rigidbody.h"
#include "components/collider.h"
#include "components/boxcollider.h"
#include "components/spherecollider.h"
#include "components/capsulecollider.h"

#include "resources/physicmaterial.h"

BulletSystem::BulletSystem(Engine *engine) :
        System(),
        m_Inited(false),
        m_pCollisionConfiguration(nullptr),
        m_pDispatcher(nullptr),
        m_pOverlappingPairCache(nullptr),
        m_pSolver(nullptr) {
    PROFILE_FUNCTION();

    Collider::registerClassFactory(this);

    RigidBody::registerClassFactory(this);
    VolumeCollider::registerClassFactory(this);

    BoxCollider::registerClassFactory(this);
    SphereCollider::registerClassFactory(this);
    CapsuleCollider::registerClassFactory(this);

    PhysicMaterial::registerClassFactory(engine->resourceSystem());
}

BulletSystem::~BulletSystem() {
    PROFILE_FUNCTION();

    for(auto &it : m_Worlds) {
        delete it.second;
    }
    delete m_pSolver;
    delete m_pOverlappingPairCache;
    delete m_pDispatcher;
    delete m_pCollisionConfiguration;

    Collider::unregisterClassFactory(this);

    RigidBody::unregisterClassFactory(this);
    VolumeCollider::unregisterClassFactory(this);

    BoxCollider::unregisterClassFactory(this);
    SphereCollider::unregisterClassFactory(this);
    CapsuleCollider::unregisterClassFactory(this);
}

bool BulletSystem::init() {
    PROFILE_FUNCTION();
    if(!m_Inited) {
        m_pCollisionConfiguration = new btDefaultCollisionConfiguration;
        m_pDispatcher = new btCollisionDispatcher(m_pCollisionConfiguration);
        m_pOverlappingPairCache = new btDbvtBroadphase;
        m_pSolver = new btSequentialImpulseConstraintSolver;
        m_pOverlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

        m_Inited = true;
    }

    return true;
}

const char *BulletSystem::name() const {
    return "Bullet Physics";
}

void BulletSystem::update(Scene *scene) {
    PROFILE_FUNCTION();

    if(Engine::isGameMode()) {
        btDynamicsWorld *world = nullptr;
        auto it = m_Worlds.find(scene->uuid());
        if(it == m_Worlds.end()) {
            world = new btDiscreteDynamicsWorld(m_pDispatcher, m_pOverlappingPairCache, m_pSolver, m_pCollisionConfiguration);
            m_Worlds[scene->uuid()] = world;
        } else {
            world = it->second;
        }

        for(auto &it : m_ObjectList) {
            Collider *body = dynamic_cast<Collider *>(it);
            if(body && body->world() == nullptr && body->actor()->scene() == scene) {
                body->setWorld(world);
            }

            body->update();
        }

        world->stepSimulation(Timer::deltaTime(), 4);
    }
}

bool BulletSystem::isThreadSafe() const {
    return true;
}
