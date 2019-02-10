#include "components/pointlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

PointLight::PointLight() {
    m_pShape = Engine::loadResource<Mesh>(".embedded/cube.fbx");

    Material *material  = Engine::loadResource<Material>(".embedded/PointLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector3("light.position",   &m_Position);
    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setVector4("light.params",     &m_Params);

    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);
}

void PointLight::draw(ICommandBuffer &buffer, int8_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {

        Matrix4 m = actor()->transform()->worldTransform();
        m_Position = Vector3(m[12], m[13], m[14]);

        Matrix4 t(Vector3(m_Position),
                  Quaternion(),
                  Vector3(m_Params.y * 2.0f, m_Params.y * 2.0f, m_Params.y * 2.0f));

        buffer.drawMesh(t, m_pShape, 0, layer, m_pMaterialInstance);
    }
}

float PointLight::radius() const {
    return m_Params.y;
}

void PointLight::setRadius(float value) {
    m_Params.y = value;
}
