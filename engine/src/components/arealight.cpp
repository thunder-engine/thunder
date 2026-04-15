#include "components/arealight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#include "pipelinecontext.h"
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
bool AreaLight::isCulled(const Frustum &frustum, const Matrix4 &viewProjection) {
    AABBox bb(m_box);
    bb.center = transform()->worldPosition();
    bb.radius = radius();
    if(frustum.contains(bb)) {
        if(m_shadows) {
            Vector4 v0(viewProjection * Vector4(bb.center, 1.0f));
            Vector2 l0(v0.x / v0.w, v0.y / v0.w);

            bb.center += frustum.m_top.normal * bb.radius;
            Vector4 v1(viewProjection * Vector4(bb.center, 1.0f));
            Vector2 l1(v1.x / v1.w, v1.y / v1.w);

            m_lod = PipelineContext::lod((l1 - l0).sqrLength());
        }

        return false;
    }

    return true;
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
