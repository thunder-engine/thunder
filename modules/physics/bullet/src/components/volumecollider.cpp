#include "components/volumecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <resources/physicmaterial.h>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>

#define MATERAIL "PhysicMaterial"

VolumeCollider::VolumeCollider() :
        m_center(0.0f),
        m_material(nullptr),
        m_dirty(false),
        m_trigger(false) {

}

VolumeCollider::~VolumeCollider() {
    if(m_world && m_collisionObject) {
        m_world->removeCollisionObject(m_collisionObject);
    }
}

void VolumeCollider::update() {
    if(m_collisionObject && m_trigger) {
        btPairCachingGhostObject *ghost = static_cast<btPairCachingGhostObject *>(m_collisionObject);
        for(int32_t i = 0; i < ghost->getOverlappingPairs().size(); i++) {
            setContact(reinterpret_cast<Collider *>(ghost->getOverlappingPairs().at(i)->getUserPointer()));
        }
    }
}

bool VolumeCollider::trigger() const {
    return m_trigger;
}

void VolumeCollider::setTrigger(bool trigger) {
    m_trigger = trigger;
}

PhysicMaterial *VolumeCollider::material() const {
    return m_material;
}

void VolumeCollider::setMaterial(PhysicMaterial *material) {
    m_material = material;
}

const Vector3 &VolumeCollider::center() const {
    return m_center;
}

void VolumeCollider::setCenter(const Vector3 center) {
    m_center = center;
    m_dirty = true;
}

void VolumeCollider::retrieveContact(const Collider *other) const {
    auto it = m_collisions.find(other->uuid());
    if(it != m_collisions.end()) {
        /// \todo Return necessary contacts data
    }
}

bool VolumeCollider::isDirty() const {
    return m_dirty;
}

void VolumeCollider::createCollider() {
    if(m_trigger) {
        if(m_collisionObject && m_world) {
            m_world->removeCollisionObject(m_collisionObject);
            delete m_collisionObject;
        }

        m_collisionObject = new btPairCachingGhostObject;
        m_collisionObject->setCollisionShape(shape());
        m_collisionObject->setUserPointer(this);

        Transform *t = transform();
        const Quaternion &q = t->worldQuaternion();
        Vector3 p = t->worldPosition();

        m_collisionObject->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                          btVector3(p.x + m_center.x, p.y + m_center.y, p.z + m_center.z)));

        m_collisionObject->setCollisionFlags(m_collisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

        m_world->addCollisionObject(m_collisionObject, btBroadphaseProxy::SensorTrigger,
                                     btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
        return;
    }

    Collider::createCollider();
}
/*!
    \internal
*/
void VolumeCollider::loadUserData(const VariantMap &data) {
    Collider::loadUserData(data);

    auto it = data.find(MATERAIL);
    if(it != data.end()) {
        setMaterial(Engine::loadResource<PhysicMaterial>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap VolumeCollider::saveUserData() const {
    VariantMap result = Collider::saveUserData();

    string ref = Engine::reference(material());
    if(!ref.empty()) {
        result[MATERAIL] = ref;
    }

    return result;
}
