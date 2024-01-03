#include "components/volumecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <resources/physicmaterial.h>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>

namespace  {
    const char *gMaterial = "PhysicMaterial";
}

/*!
    \class VolumeCollider
    \brief The VolumeCollider class represents a collider with a specified volume, and it can function as either a trigger or a physical collider.
    \inmodule Engine

    The VolumeCollider class provides functionality for defining a collider with a specific volume.
    It supports both physical and trigger collider modes, associated physics material, and can be integrated with other components in a game or simulation to enable accurate collision detection and response.
*/

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
/*!
    \internal
    Updates the state of the volume collider.
    If the collider is set as a trigger, it checks for overlapping colliders and triggers appropriate callbacks.
*/
void VolumeCollider::update() {
    if(m_collisionObject && m_trigger) {
        btPairCachingGhostObject *ghost = static_cast<btPairCachingGhostObject *>(m_collisionObject);
        for(int32_t i = 0; i < ghost->getOverlappingPairs().size(); i++) {
            setContact(reinterpret_cast<Collider *>(ghost->getOverlappingPairs().at(i)->getUserPointer()));
        }
    }
}
/*!
    Returns true if the collider is a trigger, false otherwise.
*/
bool VolumeCollider::trigger() const {
    return m_trigger;
}
/*!
     Sets whether the volume collider should function as a \a trigger.
*/
void VolumeCollider::setTrigger(bool trigger) {
    m_trigger = trigger;
}
/*!
    Returns the physics material associated with the volume collider.
*/
PhysicMaterial *VolumeCollider::material() const {
    return m_material;
}
/*!
    Sets the physics \a material for the volume collider.
*/
void VolumeCollider::setMaterial(PhysicMaterial *material) {
    m_material = material;
}
/*!
    Returns the local center of the volume collider.
*/
const Vector3 &VolumeCollider::center() const {
    return m_center;
}
/*!
    Sets the local \a center of the volume collider.
*/
void VolumeCollider::setCenter(const Vector3 center) {
    m_center = center;
    m_dirty = true;
}
/*!
    Retrieves contact information with another \a collider.
*/
void VolumeCollider::retrieveContact(const Collider *collider) const {
    auto it = m_collisions.find(collider->uuid());
    if(it != m_collisions.end()) {
        /// \todo Return necessary contacts data
    }
}
/*!
    Returns true if the collider is dirty, false otherwise.
*/
bool VolumeCollider::isDirty() const {
    return m_dirty;
}
/*!
    Creates the collision object for the volume collider.
    If the collider is a trigger, it is added as a ghost object; otherwise, it is created as a physical collider.
*/
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

    auto it = data.find(gMaterial);
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
        result[gMaterial] = ref;
    }

    return result;
}
