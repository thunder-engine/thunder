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

SpotLight::SpotLight() :
        p_ptr(new SpotLightPrivate) {

    setAngle(45.0f);

    m_pShape = Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001");

    Material *material  = Engine::loadResource<Material>(".embedded/SpotLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setVector4("light.params",     &m_Params);

    m_pMaterialInstance->setVector3("light.position",   &p_ptr->m_Position);
    m_pMaterialInstance->setVector3("light.direction",  &p_ptr->m_Direction);

    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);
}

SpotLight::~SpotLight() {
    delete p_ptr;
}

void SpotLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();
        p_ptr->m_Position = actor()->transform()->worldPosition();

        p_ptr->m_Direction = q * Vector3(0.0f, 0.0f, 1.0f);
        p_ptr->m_Direction.normalize();

        Matrix4 t(p_ptr->m_Position - p_ptr->m_Direction * radius() * 0.5f,
                  q, Vector3(m_Params.y * 2.0f, m_Params.y * 2.0f, m_Params.y));

        buffer.drawMesh(t, m_pShape, 0, layer, m_pMaterialInstance);
    }
}

float SpotLight::radius() const {
    return m_Params.y;
}

void SpotLight::setRadius(float value) {
    m_Params.y = value;
}

float SpotLight::angle() const {
    return p_ptr->m_Angle;
}

void SpotLight::setAngle(float value) {
    p_ptr->m_Angle = value;
    m_Params.z = cos(DEG2RAD * p_ptr->m_Angle);
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
