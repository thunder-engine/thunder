#include "components/spotlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"

namespace {
const char *uni_position  = "uni.position";
const char *uni_direction = "uni.direction";
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
    return m_box * actor()->transform()->worldTransform();
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

    m_box = AABBox(Vector3(0.0f, 0.0f,-0.5f) * distance, Vector3(distance * 1.5f, distance * 1.5f, distance));
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
    p.w = cos(DEG2RAD * m_angle);
    setParams(p);
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool SpotLight::drawHandles(ObjectList &selected) {
    A_UNUSED(selected);
    Transform *t = actor()->transform();

    Matrix4 z(Vector3(), Quaternion(Vector3(1, 0, 0),-90), Vector3(1.0));
    Handles::s_Color = Handles::s_Second = color();
    Handles::drawArrow(Matrix4(t->worldPosition(), t->worldRotation(), Vector3(0.25f)) * z);
    bool result = Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/spotlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
