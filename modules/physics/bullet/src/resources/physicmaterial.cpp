#include "resources/physicmaterial.h"

#include <variant.h>

#define DATA  "Data"

PhysicMaterial::PhysicMaterial() :
        m_Friction(0.5f),
        m_Restitution(0.0f),
        m_Density(1.0f) {

}
/*!
    \internal
*/
void PhysicMaterial::loadUserData(const VariantMap &data) {
    auto section = data.find(DATA);
    if(section != data.end()) {
        VariantList list = (*section).second.value<VariantList>();
        auto it = list.begin();
        m_Friction = it->toFloat();
        it++;
        m_Restitution = it->toFloat();
        it++;
        m_Density = it->toFloat();
    }
}
/*!
    \internal
*/
VariantMap PhysicMaterial::saveUserData () const {
    VariantMap result;
    VariantList data;

    data.push_back(friction());
    data.push_back(restitution());
    data.push_back(density());

    result[DATA] = data;
    return result;
}

float PhysicMaterial::friction() const {
    return m_Friction;
}

void PhysicMaterial::setFriction(float friction) {
    m_Friction = friction;
}

float PhysicMaterial::restitution() const {
    return m_Restitution;
}

void PhysicMaterial::setRestitution(float restitution) {
    m_Restitution = restitution;
}

float PhysicMaterial::density() const {
    return m_Density;
}

void PhysicMaterial::setDensity(float density) {
    m_Density = density;
}
