#include "components/pointlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

PointLight::PointLight() {
    m_pShape        = Engine::loadResource<Mesh>(".embedded/cube.fbx");

    Material *material  = Engine::loadResource<Material>(".embedded/PointLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setFloat("light.brightness",   &m_Brightness);
    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);
}

PointLight::~PointLight() {

}

void PointLight::draw(ICommandBuffer &buffer, int8_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {
        buffer.drawMesh(actor().transform()->worldTransform(), m_pShape, 0, layer, m_pMaterialInstance);
    }
}
