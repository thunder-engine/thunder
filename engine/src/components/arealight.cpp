#include "components/arealight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#include "gizmos.h"

/*!
    \class AreaLight
    \brief Point Lights works much like a real-world light bulb, emitting light in all directions.
    \inmodule Components

    To determine the emiter position AreaLight uses Transform component of the own Actor.
*/

AreaLight::AreaLight() {
    Material *material = Engine::loadResource<Material>(".embedded/AreaLight.shader");
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
float AreaLight::radius() const {
    return params().w;
}
/*!
    Changes the attenuation \a radius of the light.
*/
void AreaLight::setRadius(float radius) {
    Vector4 p = params();
    p.w = radius;
    setParams(p);

    m_box = AABBox(Vector3(), Vector3(radius * 2));
}
/*!
    Returns the source width of the light.
*/
float AreaLight::sourceWidth() const {
    return params().y;
}
/*!
    Changes the source \a width of the light.
*/
void AreaLight::setSourceWidth(float width) {
    Vector4 p = params();
    p.y = width;
    setParams(p);
}
/*!
    Returns the source height of the light.
*/
float AreaLight::sourceHeight() const {
    return params().z;
}
/*!
    Changes the source \a height of the light.
*/
void AreaLight::setSourceHeight(float height) {
    Vector4 p = params();
    p.z = height;
    setParams(p);
}
/*!
    \internal
*/
int AreaLight::lightType() const {
    return BaseLight::AreaLight;
}
/*!
    \internal
*/
AABBox AreaLight::bound() const {
    AABBox result(m_box);
    result.center += transform()->worldPosition();
    return result;
}
/*!
    \internal
*/
void AreaLight::drawGizmos() {
    Transform *t = transform();

    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/pointlight.png", color());
}
/*!
    \internal
*/
void AreaLight::drawGizmosSelected() {
    Transform *t = transform();

    Vector4 p = params();
    Gizmos::drawWireSphere(Vector3(), p.w, gizmoColor(), t->worldTransform());
    Gizmos::drawRectangle(Vector3(), Vector2(p.y, p.z), gizmoColor(), t->worldTransform());
}
