#include "components/volumecollider.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <components/actor.h>
#include <components/transform.h>

#include <resources/physicmaterial.h>

#include <log.h>

#define MATERAIL "PhysicMaterial"

#include <btBulletDynamicsCommon.h>

VolumeCollider::VolumeCollider() :
        m_Center(0.0f),
        m_pMaterial(nullptr),
        m_Dirty(false),
        m_Trigger(false) {

}

VolumeCollider::~VolumeCollider() {
    if(m_pWorld && m_pCollisionObject) {
        m_pWorld->removeCollisionObject(m_pCollisionObject);
    }
}

void VolumeCollider::update() {
    if(m_pCollisionObject && m_Trigger) {
        btPairCachingGhostObject *ghost = static_cast<btPairCachingGhostObject *>(m_pCollisionObject);
        for(int32_t i = 0; i < ghost->getOverlappingPairs().size(); i++) {
            setContact(reinterpret_cast<Collider *>(ghost->getOverlappingPairs().at(i)->getUserPointer()));
        }
    }
}

bool VolumeCollider::trigger() const {
    return m_Trigger;
}

void VolumeCollider::setTrigger(bool trigger) {
    m_Trigger = trigger;
}

PhysicMaterial *VolumeCollider::material() const {
    return m_pMaterial;
}

void VolumeCollider::setMaterial(PhysicMaterial *material) {
    m_pMaterial = material;
}

const Vector3 &VolumeCollider::center() const {
    return m_Center;
}

void VolumeCollider::setCenter(const Vector3 &center) {
    m_Center = center;
    m_Dirty = true;
}

void VolumeCollider::retrieveContact(const Collider *other) const {
    auto it = m_Collisions.find(other->uuid());
    if(it != m_Collisions.end()) {
        /// \todo Return necessary contacts data
    }
}

bool VolumeCollider::isDirty() const {
    return m_Dirty;
}

void VolumeCollider::createCollider() {
    if(m_Trigger) {
        if(m_pCollisionObject) {
            m_pWorld->removeCollisionObject(m_pCollisionObject);
            delete m_pCollisionObject;
        }

        m_pCollisionObject = new btPairCachingGhostObject;
        m_pCollisionObject->setCollisionShape(shape());
        m_pCollisionObject->setUserPointer(this);

        Actor *a = actor();
        if(a) {
            Transform *t = a->transform();
            const Quaternion &q = t->quaternion();
            Vector3 p = t->position();

            m_pCollisionObject->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                              btVector3(p.x + m_Center.x, p.y + m_Center.y, p.z + m_Center.z)));
        }
        m_pCollisionObject->setCollisionFlags(m_pCollisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

        m_pWorld->addCollisionObject(m_pCollisionObject, btBroadphaseProxy::SensorTrigger,
                                     btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
    }
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
