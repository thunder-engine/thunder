#include "resources/physicmaterial.h"

#include <variant.h>

namespace  {
    const char *gData = "Data";
}

/*!
    \class PhysicMaterial
    \brief The PhysicMaterial class represents physical properties for collision and interaction with rigid bodies.
    \inmodule Resource

    The PhysicMaterial class provides a convenient way to manage physical properties such as friction, restitution, and density for materials used in physics simulations.
    It can be associated with colliders or rigid bodies to control their behavior during interactions with other physical entities.
*/

PhysicMaterial::PhysicMaterial() :
        m_Friction(0.5f),
        m_Restitution(0.0f),
        m_Density(1.0f) {

}
/*!
    \internal
*/
void PhysicMaterial::loadUserData(const VariantMap &data) {
    auto section = data.find(gData);
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

    result[gData] = data;
    return result;
}
/*!
    Returns the coefficient of friction for the material.
*/
float PhysicMaterial::friction() const {
    return m_Friction;
}
/*!
    Sets the coefficient of \a friction for the material.
*/
void PhysicMaterial::setFriction(float friction) {
    m_Friction = friction;
}
/*!
    Returns the coefficient of restitution (bounciness) for the material.
*/
float PhysicMaterial::restitution() const {
    return m_Restitution;
}
/*!
    Sets the coefficient of \a restitution (bounciness) for the material.
*/
void PhysicMaterial::setRestitution(float restitution) {
    m_Restitution = restitution;
}
/*!
    Sets the density of the material.
*/
float PhysicMaterial::density() const {
    return m_Density;
}
/*!
    The new \a density of the material.
*/
void PhysicMaterial::setDensity(float density) {
    m_Density = density;
}
