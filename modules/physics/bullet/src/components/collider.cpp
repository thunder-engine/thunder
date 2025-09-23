#include "components/collider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <btBulletDynamicsCommon.h>

/*!
    \class Collider
    \brief The Collider class serves as a base class for defining collision shapes within a 3D physics environment.
    \inmodule Components

    The Collider class provides a foundation for creating collision shapes within a physics environment.
    It can be attached to a RigidBody for dynamic interactions or placed directly in the physics world for static collisions.
    Derived classes should implement specific collision shape creation in the shape() method.
    The class also includes methods for managing collision contacts, emitting signals, and visualizing the collider in the editor.
*/

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
/*!
    Placeholder method for updating the collider. Override this method in derived classes for specific update behavior.
*/
void Collider::update() {

}
/*!
    Returns a pointer to the attached RigidBody if one is associated with.
*/
RigidBody *Collider::attachedRigidBody() const {
    return m_rigidBody;
}
/*!
    Attaches the collider to a specific rigid \a body.
    If a RigidBody is attached, the collider will be managed by the rigid body.
*/
void Collider::setAttachedRigidBody(RigidBody *body) {
    m_rigidBody = body;
    if(m_rigidBody) {
        destroyCollider();
    }
}
/*!
    Triggers when collider enters to this volume
*/
void Collider::entered() {
    emitSignal(_SIGNAL(entered()));
}
/*!
    Triggers while collider stays in this volume
*/
void Collider::stay() {
    emitSignal(_SIGNAL(stay()));
}
/*!
    Triggers when collider exits from this volume
*/
void Collider::exited() {
    emitSignal(_SIGNAL(stay()));
}
/*!
    \internal
    Returns a pointer to the Bullet Physics dynamics world associated with the collider.
*/
btDynamicsWorld *Collider::bulletWorld() const {
    return m_world;
}
/*!
    \internal
    Sets the Bullet Physics dynamics world for the collider.
    This is necessary for adding the collider to the physics simulation.
*/
void Collider::setBulletWorld(btDynamicsWorld *world) {
    m_world = world;
    if(m_world) {
        createCollider();
    }
}
/*!
    Creates the Bullet Physics collision object associated with the collider and adds it to the physics world.
*/
void Collider::createCollider() {
    destroyCollider();

    if(m_rigidBody == nullptr) {
        btCollisionShape *s = shape();
        if(s != nullptr) {
            m_collisionObject = new btCollisionObject();
            m_collisionObject->setCollisionShape(s);
            m_collisionObject->setUserPointer(this);
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
}
/*!
    Returns a pointer to the Bullet Physics collision shape associated with the collider.
    Derived classes should implement this method to define specific collision shapes.
*/
btCollisionShape *Collider::shape() {
    return m_collisionShape;
}
/*!
    \internal
*/
btCollisionObject *Collider::collisionObject() {
    return m_collisionObject;
}
/*!
    \internal
    Destroys the Bullet Physics collision shape associated with the collider.
*/
void Collider::destroyShape() {
    delete m_collisionShape;
    m_collisionShape = nullptr;
}
/*!
    \internal
    Destroys the Bullet Physics collision object associated with the collider and removes it from the physics world.
*/
void Collider::destroyCollider() {
    if(m_collisionObject && m_world) {
        m_world->removeCollisionObject(m_collisionObject);
        delete m_collisionObject;
        m_collisionObject = nullptr;
    }
}
/*!
    Marks all current collision contacts as dirty, indicating that they should be checked for updates.
*/
void Collider::dirtyContacts() {
    for(auto &it : m_collisions) {
        it.second = true;
    }
}
/*!
    Cleans up stale collision contacts and emits signals for collisions that have ended.
*/
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
/*!
    Sets a new collision contact with another \a collider.
    Emits appropriate signals based on whether the contact is new, sustained, or ended.
*/
void Collider::setContact(Collider *collider) {
    if(collider == nullptr) {
        return;
    }
    bool result = true;
    for(auto &it : m_collisions) {
        if(it.first == collider->uuid()) {
            stay();
            it.second = false;
            result = false;
            break;
        }
    }
    if(result) {
        entered();
        m_collisions[collider->uuid()] = false;
    }
}
/*!
    \internal
    Returns the color used for visualizing the collider in editor Gizmos.
*/
Vector4 Collider::gizmoColor() const {
    return Vector4(0.5f, 1.0f, 0.5f, 1.0f);
}
