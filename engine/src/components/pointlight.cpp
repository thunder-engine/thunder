#include "components/pointlight.h"

#include "camera.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#include "pipelinecontext.h"
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
bool PointLight::isCulled(const Frustum &frustum, const Matrix4 &viewProjection) {
    AABBox bb;
    bb.center = transform()->worldPosition();
    bb.radius = attenuationRadius();
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
void PointLight::drawGizmos() {
    Gizmos::drawIcon(transform()->worldPosition(), Vector2(0.5f), ".embedded/pointlight.png", color());
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
