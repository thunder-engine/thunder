#include "components/volumecollider.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <components/actor.h>
#include <components/transform.h>

#include <resources/physicmaterial.h>

#define MATERAIL "PhysicMaterial"

VolumeCollider::VolumeCollider() :
        m_Center(0.0f),
        m_pMaterial(nullptr) {

}

VolumeCollider::~VolumeCollider() {
    if(m_pWorld && m_pCollisionObject) {
        m_pWorld->removeCollisionObject(m_pCollisionObject);
    }
}

void VolumeCollider::update() {
    if(m_pCollisionObject) {
        for(auto &it : m_Collisions) {
            it.second = true;
        }

        btPairCachingGhostObject *ghost = static_cast<btPairCachingGhostObject *>(m_pCollisionObject);
        for(uint32_t i = 0; i < ghost->getOverlappingPairs().size(); i++) {
            Collider *other = reinterpret_cast<Collider *>(ghost->getOverlappingPairs().at(i)->getUserPointer());
            bool result = true;
            for(auto &it : m_Collisions) {
                if(it.first == other->uuid()) {
                    emitSignal(_SIGNAL(stay()));
                    it.second = false;
                    result = false;
                    break;
                }
            }
            if(result) {
                emitSignal(_SIGNAL(entered()));
                m_Collisions[other->uuid()] = false;
            }
        }

        auto it = m_Collisions.begin();
        while(it != m_Collisions.end()) {
            if(it->second == true) {
                emitSignal(_SIGNAL(exited()));
                it = m_Collisions.erase(it);
                m_pCollisionObject->activate(true);
            } else {
                ++it;
            }
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
}

void VolumeCollider::retrieveContact (const Collider *other) const {
    auto it = m_Collisions.find(other->uuid());
    if(it != m_Collisions.end()) {
        /// \todo Return necessary contacts data
    }
}

void VolumeCollider::createCollider() {
    if(m_Trigger) {
        m_pCollisionObject = new btPairCachingGhostObject;
        m_pCollisionObject->setCollisionShape(shape());
        m_pCollisionObject->setUserPointer(this);

        Actor *a = actor();
        if(a) {
            Transform *t = a->transform();
            const Quaternion &q = t->quaternion();
            Vector3 p = t->position();

            m_pCollisionObject->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w), btVector3(p.x, p.y, p.z)));
        }
        m_pCollisionObject->setCollisionFlags(m_pCollisionObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

        if(m_pCollisionObject) {
            m_pWorld->addCollisionObject(m_pCollisionObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
        }
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
