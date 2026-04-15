#include "components/spotlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "resources/material.h"

#include "pipelinecontext.h"
#include "gizmos.h"

namespace {
    const char *uniBias("bias");
    const char *uniMatrix("matrix");
}

/*!
    \class SpotLight
    \brief A Spot Light emits light from a single point in a cone shape.
    \inmodule Components

    To determine the emitter position and emit direction SpotLight uses Transform component of the own Actor.
*/

SpotLight::SpotLight() :
        m_angle(0.0f) {

    setOuterAngle(45.0f);

    Material *material = Engine::loadResource<Material>(".embedded/SpotLight.shader");
    if(material) {
        setMaterial(material->createInstance());
    }
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
    m_dirty = true;
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
    m_dirty = true;
}
/*!
    \internal
*/
void SpotLight::cleanDirty() {
    Transform *t = transform();
    m_hash = t->hash();

    Matrix4 wt(t->worldTransform());

    m_viewFrustum.resize(1);
    m_cropMatrix.resize(1);

    float zNear = 0.1f;
    float zFar = attenuationDistance();

    m_viewFrustum[0] = Camera::frustum(false, m_angle, 1.0f, wt.position(), t->worldQuaternion(), zNear, zFar);
    m_cropMatrix[0] = Matrix4::perspective(outerAngle(), 1.0f, zNear, zFar) * wt.inverse();

    if(m_materialInstance) {
        Matrix4 matrix = s_scale * m_cropMatrix[0];

        Vector4 bias;
        m_materialInstance->setVector4(uniBias, &bias);
        m_materialInstance->setMatrix4(uniMatrix, &matrix);
    }

    m_dirty = false;
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
bool SpotLight::isCulled(const Frustum &frustum, const Matrix4 &viewProjection) {
    Transform *t = transform();

    Quaternion q(t->worldQuaternion());

    Vector3 position(t->worldPosition());
    Vector3 direction(q * Vector3(0.0f, 0.0f, 1.0f));

    float distance = attenuationDistance();
    float diameter = tan(DEG2RAD * m_angle) * distance;
    AABBox bb;
    bb.center = position - direction * distance * 0.5f;
    bb.radius = MAX(diameter, distance) * 0.5f;
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
int SpotLight::tilesCount() const {
    return 1;
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
    Gizmos::drawCircle(t->worldQuaternion() * Vector3(0.0f, 0.0f, -distance), radius, gizmoColor(), m);

    Gizmos::drawLines({Vector3(),
                       Vector3( 0.0f, radius, -distance),
                       Vector3( 0.0f,-radius, -distance),
                       Vector3( radius, 0.0f, -distance),
                       Vector3(-radius, 0.0f, -distance)},
                      {0, 1, 0, 2, 0, 3, 0, 4}, gizmoColor(), t->worldTransform());
}
