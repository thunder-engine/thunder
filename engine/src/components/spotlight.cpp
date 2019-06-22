#include "components/spotlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

class SpotLightPrivate {
public:
    SpotLightPrivate() :
            m_Angle(45.0f) {
    }

    Vector3                     m_Position;

    Vector3                     m_Direction;

    float                       m_Angle;
};
/*!
    \class SpotLight
    \brief A Spot Light emits light from a single point in a cone shape.
    \inmodule Engine

    To determine the emitter position and emit direction SpotLight uses Transform component of the own Actor.
*/

SpotLight::SpotLight() :
        p_ptr(new SpotLightPrivate) {

    setAngle(45.0f);

    setShape(Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001"));

    Material *material = Engine::loadResource<Material>(".embedded/SpotLight.mtl");
    MaterialInstance *instance = material->createInstance();

    instance->setVector3("light.position",   &p_ptr->m_Position);
    instance->setVector3("light.direction",  &p_ptr->m_Direction);

    setMaterial(instance);
}

SpotLight::~SpotLight() {
    delete p_ptr;
}
/*!
    \internal
*/
void SpotLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & ICommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();
        p_ptr->m_Position = actor()->transform()->worldPosition();

        p_ptr->m_Direction = q * Vector3(0.0f, 0.0f, 1.0f);
        p_ptr->m_Direction.normalize();

        Vector4 p = params();

        Matrix4 t(p_ptr->m_Position - p_ptr->m_Direction * distance() * 0.5f,
                  q, Vector3(p.y * 2.0f, p.y * 2.0f, p.y));

        buffer.drawMesh(t, mesh, layer, instance);
    }
}
/*!
    Returns the attenuation distance of the light cone.
*/
float SpotLight::distance() const {
    return params().y;
}
/*!
    Changes the attenuation \a distance of the light cone.
*/
void SpotLight::setDistance(float distance) {
    Vector4 p = params();
    p.y = distance;
    setParams(p);
}
/*!
    Returns the angle of the light cone in degrees.
*/
float SpotLight::angle() const {
    return p_ptr->m_Angle;
}
/*!
    Changes the \a angle of the light cone in degrees.
*/
void SpotLight::setAngle(float angle) {
    p_ptr->m_Angle = angle;
    Vector4 p = params();
    p.z = cos(DEG2RAD * p_ptr->m_Angle);
    setParams(p);
}

#ifdef NEXT_SHARED
#include "handles.h"

bool SpotLight::drawHandles() {
    Vector3 pos = actor()->transform()->position();

    Handles::s_Color = Handles::s_Second = color();
    bool result = Handles::drawBillboard(pos, Vector2(1.0), Engine::loadResource<Texture>(".embedded/spotlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
