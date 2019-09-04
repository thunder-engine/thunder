#include "resources/physicmaterial.h"

#include <variant.h>

#define DATA  "Data"

PhysicMaterial::PhysicMaterial() :
        m_Friction(0.5f),
        m_Bounciness(0.0f) {

}

void PhysicMaterial::loadUserData(const VariantMap &data) {
    auto section = data.find(DATA);
    if(section != data.end()) {
        VariantList list = (*section).second.value<VariantList>();
        auto it = list.begin();
        m_Friction = it->toFloat();
        it++;
        m_Bounciness = it->toFloat();
    }
}

float PhysicMaterial::friction() const {
    return m_Friction;
}

void PhysicMaterial::setFriction(float friction) {
    m_Friction = friction;
}

float PhysicMaterial::bounciness() const {
    return m_Bounciness;
}

void PhysicMaterial::setBounciness(float bounciness) {
    m_Bounciness = bounciness;
}
