#include "components/directlight.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

#define MAX_LODS 4

DirectLight::DirectLight() {
    m_Shadows       = false;
    m_Brightness    = 1.0f;
    m_Bias          = 0.001f;
    m_Color         = Vector4(1.0f);

    m_pPlane        = Engine::loadResource<Mesh>(".embedded/plane.fbx");

    m_pMatrix       = new Matrix4[MAX_LODS];
    m_pTiles        = new Vector4[MAX_LODS];

    m_pMaterial     = Engine::loadResource<Material>(".embedded/VSM.mtl");
    m_pMaterialInstance = m_pMaterial->createInstance();

    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setFloat("light.brightness",   &m_Brightness);
    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);

    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);
    m_pMaterialInstance->setVector4("light.lod",        &m_NormalizedDistance);

    m_pMaterialInstance->setMatrix4("light.matrix",     m_pMatrix, MAX_LODS);
    m_pMaterialInstance->setVector4("light.tiles",      m_pTiles,  MAX_LODS);
}

DirectLight::~DirectLight() {
    delete m_pMatrix;
    delete m_pTiles;
}

void DirectLight::draw(ICommandBuffer &buffer, int8_t layer) {
    if(m_pPlane && m_pMaterial && (layer & ICommandBuffer::LIGHT)) {
        buffer.setViewProjection(Matrix4(), Matrix4::ortho( 0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
        buffer.drawMesh(Matrix4(), m_pPlane, 0, layer, m_pMaterialInstance);
    }
}

bool DirectLight::castShadows() const {
    return (m_Shadows == 1.0f);
}

void DirectLight::setCastShadows(bool shadows) {
    m_Shadows   = (shadows) ? 1.0f : 0.0f;
}

float DirectLight::brightness() const {
    return m_Brightness;
}

void DirectLight::setBrightness(const float brightness) {
    m_Brightness= brightness;
}

Vector4 DirectLight::color() const {
    return m_Color;
}
void DirectLight::setColor(const Vector4 &color) {
    m_Color = color;
}

float DirectLight::bias() const {
    return m_Bias;
}

void DirectLight::setBias(const float bias) {
    m_Bias = bias;
}
