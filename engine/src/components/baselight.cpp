#include "baselight.h"

#include "components/actor.h"
#include "components/transform.h"

BaseLight::BaseLight() :
        m_Shadows(0.0f),
        m_Bias(0.001f),
        m_Params(1.0f, 1.0f, 0.5f, 1.0f),
        m_Color(1.0f),
        m_pShape(nullptr),
        m_pMaterialInstance(nullptr) {

}

BaseLight::~BaseLight() {

}

bool BaseLight::castShadows() const {
    return (m_Shadows == 1.0f);
}

void BaseLight::setCastShadows(const bool shadows) {
    m_Shadows = (shadows) ? 1.0f : 0.0f;
}

float BaseLight::brightness() const {
    return m_Params.x;
}

void BaseLight::setBrightness(const float brightness) {
    m_Params.x    = brightness;
}

Vector4 BaseLight::color() const {
    return m_Color;
}
void BaseLight::setColor(const Vector4 &color) {
    m_Color = color;
}

float BaseLight::bias() const {
    return m_Bias;
}

void BaseLight::setBias(const float bias) {
    m_Bias = bias;
}
