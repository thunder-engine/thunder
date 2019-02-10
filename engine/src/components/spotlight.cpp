#include "components/spotlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

SpotLight::SpotLight() {

    setAngle(45.0f);

    m_pShape = Engine::loadResource<Mesh>(".embedded/cube.fbx");

    Material *material  = Engine::loadResource<Material>(".embedded/SpotLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setVector4("light.params",     &m_Params);

    m_pMaterialInstance->setVector3("light.position",   &m_Position);
    m_pMaterialInstance->setVector3("light.direction",  &m_Direction);

    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);
}

void SpotLight::draw(ICommandBuffer &buffer, int8_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();
        m_Position = actor()->transform()->worldPosition();

        m_Direction = q * Vector3(0.0f, 0.0f, 1.0f);
        m_Direction.normalize();

        Matrix4 t(m_Position - m_Direction * radius() * 0.5f,
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
    return m_Angle;
}

void SpotLight::setAngle(float value) {
    m_Angle = value;
    m_Params.z = cos(DEG2RAD * m_Angle);
}
