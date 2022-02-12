#include "components/charactercontroller.h"

#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <btBulletDynamicsCommon.h>

#include <components/actor.h>
#include <components/transform.h>

//btCollisionShape *CharacterController::shape() {
//    if(m_pCollisionShape == nullptr) {
//        m_pCollisionShape = new btCapsuleShape(m_Radius, m_Height);
//
//        Transform *t = actor()->transform();
//
//        Vector3 p = t->scale();
//        m_pCollisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));
//    }
//
//    if(m_pCharacter == nullptr) {
//        m_pCharacter = new btKinematicCharacterController(nullptr, static_cast<btConvexShape *>(m_pCollisionShape), 0.05f);
//    }
//
//    return m_pCollisionShape;
//}


CharacterController::CharacterController() {

}

float CharacterController::height() const {
    return m_Height;
}

void CharacterController::setHeight(float height) {
    m_Height = height;
}

float CharacterController::radius() const {
    return m_Radius;
}

void CharacterController::setRadius(float radius) {
    m_Radius = radius;
}
