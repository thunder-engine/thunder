#include "components/spotlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#include "gizmos.h"

namespace {
const char *uni_position  = "position";
const char *uni_direction = "direction";
};

/*!
    \class SpotLight
    \brief A Spot Light emits light from a single point in a cone shape.
    \inmodule Engine

    To determine the emitter position and emit direction SpotLight uses Transform component of the own Actor.
*/

SpotLight::SpotLight() :
        m_angle(0.0f) {

    setOuterAngle(45.0f);

    Material *material = Engine::loadResource<Material>(".embedded/SpotLight.shader");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        setMaterial(instance);
    }
}
/*!
    \internal
*/
int SpotLight::lightType() const {
    return BaseLight::SpotLight;
}
/*!
    \internal
*/
AABBox SpotLight::bound() const {
    float distance = params().y;
    float diameter = tan(DEG2RAD * m_angle) * distance;
    AABBox aabb(Vector3(0.0f, 0.0f, 0.5f) * distance, Vector3(diameter, diameter, distance));
    return aabb * transform()->worldTransform();
}
/*!
    Returns the attenuation distance of the light cone.
*/
float SpotLight::attenuationDistance() const {
    return params().y;
}
/*!
    Changes the attenuation \a distance of the light cone.
*/
void SpotLight::setAttenuationDistance(float distance) {
    Vector4 p = params();
    p.y = distance;
    setParams(p);
}
/*!
    Returns the angle of the light cone in degrees.
*/
float SpotLight::outerAngle() const {
    return m_angle;
}
/*!
    Changes the \a angle of the light cone in degrees.
*/
void SpotLight::setOuterAngle(float angle) {
    m_angle = angle;
    Vector4 p = params();
    p.w = cos(DEG2RAD * m_angle * 0.5f);
    setParams(p);
}
/*!
    \internal
*/
void SpotLight::drawGizmos() {
    Transform *t = transform();

    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/spotlight.png", color());
}
/*!
    \internal
*/
void SpotLight::drawGizmosSelected() {
    Transform *t = transform();

    float distance = attenuationDistance();
    float radius = tan(DEG2RAD * m_angle * 0.5f) * distance;

    Matrix4 m(t->worldPosition(), t->worldQuaternion() * Quaternion(Vector3(1, 0, 0), 90), Vector3(1.0f));
    Gizmos::drawCircle(t->worldQuaternion() * Vector3(0.0f, 0.0f, distance), radius, gizmoColor(), m);

    Gizmos::drawLines({Vector3(),
                       Vector3( 0.0f, radius, distance),
                       Vector3( 0.0f,-radius, distance),
                       Vector3( radius, 0.0f, distance),
                       Vector3(-radius, 0.0f, distance)},
                      {0, 1, 0, 2, 0, 3, 0, 4}, gizmoColor(), t->worldTransform());
}
