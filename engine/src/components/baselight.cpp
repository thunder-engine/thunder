#include "baselight.h"

#include "components/actor.h"
#include "components/transform.h"

BaseLight::BaseLight() :
        m_Shadows(false),
        m_Brightness(1.0f),
        m_Bias(0.001f),
        m_Color(1.0f),
        m_pShape(nullptr),
        m_pMaterialInstance(nullptr) {

}

BaseLight::~BaseLight() {

}

bool BaseLight::castShadows() const {
    return (m_Shadows == 1.0f);
}

void BaseLight::setCastShadows(bool shadows) {
    m_Shadows   = (shadows) ? 1.0f : 0.0f;
}

float BaseLight::brightness() const {
    return m_Brightness;
}

void BaseLight::setBrightness(const float brightness) {
    m_Brightness    = brightness;
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
