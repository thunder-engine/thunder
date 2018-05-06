#include "components/directlight.h"

DirectLight::DirectLight() {
    m_Shadows       = false;
    m_Brightness    = 1.0f;
    m_Bias          = 0.001f;
    m_Color         = Vector4(1.0f);
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
