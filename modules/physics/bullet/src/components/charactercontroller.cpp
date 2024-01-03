#include "components/charactercontroller.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>

/*!
    \class CharacterController
    \brief The CharacterController class represents a kinematic character controller for controlling character movement in a 3D physics environment.
    \inmodule Engine

    The CharacterController class provides methods to control the movement and properties of a character in a 3D physics environment.
*/

CharacterController::CharacterController() :
        m_character(nullptr),
        m_ghostObject(new btPairCachingGhostObject()),
        m_height(2.0f),
        m_radius(0.5f),
        m_skin(0.08f),
        m_slope(45.0f),
        m_step(0.3f),
        m_dirty(false) {

    m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    m_ghostObject->setUserPointer(this);
}

CharacterController::~CharacterController() {
    destroyCharacter();
    destroyShape();

    if(m_world) {
        m_world->removeCollisionObject(m_ghostObject);
    }
}
/*!
    \internal
    Updates the position of the character controller based on the ghost object's transformation.
*/
void CharacterController::update() {
    btVector3 &p = m_ghostObject->getWorldTransform().getOrigin();
    Vector3 position(p.x() - m_center.x, p.y() - m_center.y, p.z() - m_center.z);

    Transform *t = transform();
    Transform *parent = t->parentTransform();
    if(parent) {
        t->setPosition(parent->worldTransform().inverse() * position);
    } else {
        t->setPosition(position);
    }
}
/*!
    Returns the height of the character controller's capsule shape.
*/
float CharacterController::height() const {
    return m_height;
}
/*!
    Sets the \a height of the character controller's capsule shape.
*/
void CharacterController::setHeight(float height) {
    m_height = height;
    m_dirty = true;
}
/*!
    Returns the radius of the character controller's capsule shape.
*/
float CharacterController::radius() const {
    return m_radius;
}
/*!
    Sets the \a radius of the character controller's capsule shape.
*/
void CharacterController::setRadius(float radius) {
    m_radius = radius;
    m_dirty = true;
}
/*!
    Returns the slope limit angle for the character controller.
*/
float CharacterController::slopeLimit() const {
    return m_slope;
}
/*!
    Sets the slope \a limit angle for the character controller.
*/
void CharacterController::setSlopeLimit(float limit) {
    m_slope = limit;
    if(m_character) {
        m_character->setMaxSlope(m_slope);
    }
}
/*!
    Returns the maximum height of steps that the character controller can climb.
*/
float CharacterController::stepOffset() const {
    return m_step;
}
/*!
    Sets the maximum \a height of steps that the character controller can climb.
*/
void CharacterController::setStepOffset(float height) {
    m_step = height;
    if(m_character) {
        m_character->setStepHeight(height);
    }
}
/*!
    Returns the skin width of the character controller.
*/
float CharacterController::skinWidth() const {
    return m_skin;
}
/*!
    Sets the skin \a width of the character controller.
*/
void CharacterController::setSkinWidth(float width) {
    m_skin = width;
    m_dirty = true;
}
/*!
    Returns the local center of the character controller.
*/
Vector3 CharacterController::center() const {
    return m_center;
}
/*!
    Sets the local \a center of the character controller.
*/
void CharacterController::setCenter(const Vector3 center) {
    m_center = center;
    m_dirty = true;
}
/*!
    Returns the gravity vector applied to the character controller.
*/
Vector3 CharacterController::gravity() const {
    if(m_character) {
        btVector3 g = m_character->getGravity();
        return Vector3(g.x(), g.y(), g.z());
    }
    return Vector3();
}
/*!
    Sets the gravity \a vector applied to the character controller.
*/
void CharacterController::setGravity(const Vector3 gravity) {
    if(m_character) {
        m_character->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }
}
/*!
    Moves the character controller in the specified direction.
*/
void CharacterController::move(const Vector3 &direction) {
    if(m_character) {
        m_character->setWalkDirection(btVector3(direction.x, direction.y, direction.z));
    }
}
/*!
    Returns true if the character controller is currently grounded (on the floor); otherwise, returns false.
*/
bool CharacterController::isGrounded() const {
    return (m_character) ? m_character->onGround() : false;
}
/*!
    \internal
    Creates the collider for the character controller, adds it to the physics world, and sets up various properties such as slope limit and step offset.
*/
void CharacterController::createCollider() {
    if(m_character == nullptr) {
        m_character = new btKinematicCharacterController(m_ghostObject, static_cast<btConvexShape *>(shape()), m_skin);

        m_character->setMaxSlope(m_slope);
        m_character->setStepHeight(m_step);
        m_character->setUp(btVector3(0, 1, 0));
        m_character->setGravity(btVector3(0, 0, 0));

        Transform *t = transform();
        const Quaternion q = t->worldQuaternion();
        Vector3 p = t->worldPosition();

        m_ghostObject->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                     btVector3(p.x + m_center.x, p.y + m_center.y, p.z + m_center.z)));
    }

    if(m_character && m_world) {
        m_world->addCollisionObject(m_ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
        m_world->addAction(m_character);
    }
}
/*!
    \internal
    Returns the Bullet Physics collision shape associated with the character controller's capsule shape.
    If the shape does not exist or if the properties have been changed, it creates a new shape.
*/
btCollisionShape *CharacterController::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btCapsuleShape(m_radius, m_height - m_radius * 2.0f);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_ghostObject->setCollisionShape(m_collisionShape);
    }

    return m_collisionShape;
}
/*!
    \internal
    Destroys the character controller, removing it from the physics world.
*/
void CharacterController::destroyCharacter() {
    if(m_character && m_world) {
        m_world->removeAction(m_character);
    }
    delete m_character;
    m_character = nullptr;
}
/*!
    \internal
    Draws a wireframe representation of the character controller's capsule shape in the scene for visualization purposes.
    This method is typically called when the actor is selected in the editor.
*/
void CharacterController::drawGizmosSelected() {
    Transform *t = transform();
    Gizmos::drawWireCapsule(m_center, m_radius, m_height, gizmoColor(), t->worldTransform());
}
