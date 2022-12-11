#include "components/charactercontroller.h"

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>

#include <components/actor.h>
#include <components/transform.h>

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

float CharacterController::height() const {
    return m_height;
}

void CharacterController::setHeight(float height) {
    m_height = height;
    m_dirty = true;
}

float CharacterController::radius() const {
    return m_radius;
}

void CharacterController::setRadius(float radius) {
    m_radius = radius;
    m_dirty = true;
}

float CharacterController::slopeLimit() const {
    return m_slope;
}

void CharacterController::setSlopeLimit(float limit) {
    m_slope = limit;
    if(m_character) {
        m_character->setMaxSlope(m_slope);
    }
}

float CharacterController::stepOffset() const {
    return m_step;
}

void CharacterController::setStepOffset(float step) {
    m_step = step;
    if(m_character) {
        m_character->setStepHeight(step);
    }
}

float CharacterController::skinWidth() const {
    return m_skin;
}

void CharacterController::setSkinWidth(float width) {
    m_skin = width;
    m_dirty = true;
}

Vector3 CharacterController::center() const {
    return m_center;
}

void CharacterController::setCenter(const Vector3 center) {
    m_center = center;
    m_dirty = true;
}

void CharacterController::move(const Vector3 &vector) {
    if(m_character) {
        m_character->setWalkDirection(btVector3(vector.x, vector.y, vector.z));
    }
}

bool CharacterController::isGrounded() const {
    return (m_character) ? m_character->onGround() : false;
}

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

btCollisionShape *CharacterController::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btCapsuleShape(m_radius, m_height - m_radius * 2.0f);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_ghostObject->setCollisionShape(m_collisionShape);
    }

    return m_collisionShape;
}

void CharacterController::destroyCharacter() {
    if(m_character && m_world) {
        m_world->removeAction(m_character);
    }
    delete m_character;
    m_character = nullptr;
}

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

bool CharacterController::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        Transform *t = transform();
        Handles::drawCapsule(t->worldPosition() + t->worldQuaternion() * m_center, t->worldRotation(), m_radius, m_height);
    }

    return false;
}
#endif
