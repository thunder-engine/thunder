#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

#define MAX_LODS 4

DirectLight::DirectLight() {
    m_pShape        = Engine::loadResource<Mesh>(".embedded/plane.fbx");

    Material *material  = Engine::loadResource<Material>(".embedded/DirectLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setVector4("light.lod",        &m_NormalizedDistance);
    m_pMaterialInstance->setVector4("light.params",     &m_Params);

    m_pMaterialInstance->setVector3("light.direction",  &m_Direction);

    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);

    m_pMatrix       = new Matrix4[MAX_LODS];
    m_pTiles        = new Vector4[MAX_LODS];

    m_pMaterialInstance->setMatrix4("light.matrix",     m_pMatrix, MAX_LODS);
    m_pMaterialInstance->setVector4("light.tiles",      m_pTiles,  MAX_LODS);
}

DirectLight::~DirectLight() {
    delete m_pMatrix;
    delete m_pTiles;
}

void DirectLight::draw(ICommandBuffer &buffer, int8_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {
        Matrix4 m = actor()->transform()->worldTransform();

        m_Direction = m.rotation() * Vector3(0.0f, 0.0f, 1.0f);

        buffer.setScreenProjection();
        buffer.drawMesh(Matrix4(), m_pShape, 0, layer, m_pMaterialInstance);
        buffer.resetViewProjection();
    }
}

Vector4 &DirectLight::normalizedDistance() {
    return m_NormalizedDistance;
}

Vector4 *DirectLight::tiles() {
    return m_pTiles;
}

Matrix4 *DirectLight::matrix() {
    return m_pMatrix;
}
