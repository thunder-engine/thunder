#include "components/collider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <btBulletDynamicsCommon.h>

Collider::Collider() :
        m_collisionShape(nullptr),
        m_collisionObject(nullptr),
        m_world(nullptr),
        m_rigidBody(nullptr) {

}

Collider::~Collider() {
    destroyShape();
    destroyCollider();
}

void Collider::update() {

}

RigidBody *Collider::attachedRigidBody() const {
    return m_rigidBody;
}

void Collider::setAttachedRigidBody(RigidBody *body) {
    m_rigidBody = body;
    if(m_rigidBody) {
        destroyCollider();
    }
}

btDynamicsWorld *Collider::world() const {
    return m_world;
}

void Collider::setWorld(btDynamicsWorld *world) {
    m_world = world;
    if(m_world) {
        createCollider();
    }
}

void Collider::createCollider() {
    destroyCollider();

    if(m_rigidBody == nullptr) {
        m_collisionObject = new btCollisionObject();
        m_collisionObject->setCollisionShape(shape());
        if(m_world) {
            Transform *t = transform();

            Quaternion q = t->worldQuaternion();
            Vector3 p = t->worldPosition();

            m_collisionObject->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                             btVector3(p.x, p.y, p.z)));

            m_world->addCollisionObject(m_collisionObject);
        }
    }
}

btCollisionShape *Collider::shape() {
    return m_collisionShape;
}

void Collider::destroyShape() {
    delete m_collisionShape;
    m_collisionShape = nullptr;
}

void Collider::destroyCollider() {
    if(m_collisionObject && m_world) {
        m_world->removeCollisionObject(m_collisionObject);
        delete m_collisionObject;
        m_collisionObject = nullptr;
    }
}

void Collider::dirtyContacts() {
    for(auto &it : m_collisions) {
        it.second = true;
    }
}

void Collider::cleanContacts() {
    auto it = m_collisions.begin();
    while(it != m_collisions.end()) {
        if(it->second == true) {
            emitSignal(_SIGNAL(exited()));
            it = m_collisions.erase(it);
            if(m_collisionObject) {
                m_collisionObject->activate(true);
            }
        } else {
            ++it;
        }
    }
}

void Collider::setContact(Collider *other) {
    bool result = true;
    for(auto &it : m_collisions) {
        if(it.first == other->uuid()) {
            emitSignal(_SIGNAL(stay()));
            it.second = false;
            result = false;
            break;
        }
    }
    if(result) {
        emitSignal(_SIGNAL(entered()));
        m_collisions[other->uuid()] = false;
    }
}
