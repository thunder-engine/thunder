#include "components/pointlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#include "gizmos.h"

/*!
    \class PointLight
    \brief Point Lights works much like a real-world light bulb, emitting light in all directions.
    \inmodule Components

    To determine the emiter position PointLight uses Transform component of the own Actor.
*/

PointLight::PointLight() {
    Material *material = Engine::loadResource<Material>(".embedded/PointLight.shader");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        setMaterial(instance);
    }

    Vector4 p = params();
    p.y = 0.1f;
    setParams(p);
}
/*!
    Returns the attenuation radius of the light.
*/
float PointLight::attenuationRadius() const {
    return params().w;
}
/*!
    Changes the attenuation \a radius of the light.
*/
void PointLight::setAttenuationRadius(float radius) {
    Vector4 p = params();
    p.w = radius;
    setParams(p);

    m_box = AABBox(Vector3(), Vector3(radius * 2));
}
/*!
    Returns the source radius of the light.
*/
float PointLight::sourceRadius() const {
    return params().y;
}
/*!
    Changes the source \a radius of the light.
*/
void PointLight::setSourceRadius(float radius) {
    Vector4 p = params();
    p.y = radius;
    setParams(p);
}
/*!
    Returns the source length of the light.
*/
float PointLight::sourceLength() const {
    return params().z;
}
/*!
    Changes the source \a length of the light.
*/
void PointLight::setSourceLength(float length) {
    Vector4 p = params();
    p.z = length;
    setParams(p);
}
/*!
    \internal
*/
int PointLight::lightType() const {
    return BaseLight::PointLight;
}
/*!
    \internal
*/
AABBox PointLight::bound() const {
    AABBox result(m_box);
    result.center += transform()->worldPosition();
    return result;
}
/*!
    \internal
*/
void PointLight::drawGizmos() {
    Transform *t = transform();

    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/pointlight.png", color());
}
/*!
    \internal
*/
void PointLight::drawGizmosSelected() {
    Transform *t = transform();

    Vector4 p = params();
    Gizmos::drawWireSphere(Vector3(), p.w, gizmoColor(), t->worldTransform());
    Gizmos::drawWireCapsule(Vector3(), p.y, p.z + p.y * 2.0f, gizmoColor(), t->worldTransform());
}
