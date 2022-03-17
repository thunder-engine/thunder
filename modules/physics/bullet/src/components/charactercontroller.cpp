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
        m_step(0.3f) {

    m_ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    m_ghostObject->setUserPointer(this);
}

CharacterController::~CharacterController() {
    destroyCharacter();

    if(m_world) {
        m_world->removeCollisionObject(m_ghostObject);
    }
}

void CharacterController::update() {
    btTransform &transform = m_ghostObject->getWorldTransform();
    btVector3 &p = transform.getOrigin();
    Vector3 position(p.x(), p.y(), p.z());

    Transform *t = actor()->transform();

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

    destroyCharacter();
    destroyShape();

    createCollider();
}

float CharacterController::radius() const {
    return m_radius;
}

void CharacterController::setRadius(float radius) {
    m_radius = radius;

    destroyCharacter();
    destroyShape();

    createCollider();
}

float CharacterController::slopeLimit() const {
    return m_slope;
}

void CharacterController::setSlopeLimit(float limit) {
    m_slope = limit;
    if(m_character == nullptr) {
        m_character->setMaxSlope(m_slope);
    }
}

float CharacterController::stepOffset() const {
    return m_step;
}

void CharacterController::setStepOffset(float step) {
    m_step = step;
    if(m_character == nullptr) {
        m_character->setStepHeight(step);
    }
}

float CharacterController::skinWidth() const {
    return m_skin;
}

void CharacterController::setSkinWidth(float width) {
    m_skin = width;

    destroyCharacter();
    createCollider();
}

Vector3 CharacterController::center() const {
    return m_center;
}

void CharacterController::setCenter(const Vector3 center) {
    m_center = center;
}

void CharacterController::move(const Vector3 &vector) {
    if(m_character) {
        m_character->setWalkDirection(btVector3(vector.x, vector.y, vector.z));
    }
}

void CharacterController::jump(const Vector3 &vector) {
    if(m_character) {
        m_character->jump(btVector3(vector.x, vector.y, vector.z));
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

        Transform *t = actor()->transform();
        if(t) {
            Vector3 center = m_center;
            center += actor()->transform()->position();

            btTransform &transform = m_ghostObject->getWorldTransform();
            transform.setOrigin(btVector3(center.x, center.y, center.z));
        }
    }

    if(m_character && m_world) {
        m_world->addCollisionObject(m_ghostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::AllFilter);/*btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter*/
        m_world->addAction(m_character);
        m_character->setGravity(m_world->getGravity());
    }
}

btCollisionShape *CharacterController::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btCapsuleShape(m_radius, m_height);

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
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
        Transform *t = actor()->transform();
        Handles::drawCapsule(t->worldPosition() + t->worldQuaternion() * m_center, t->worldRotation(), m_radius, m_height);
    }
    return false;
}
#endif
