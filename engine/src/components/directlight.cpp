#include "components/directlight.h"

DirectLight::DirectLight() {
    m_Shadows       = false;
    m_Brightness    = 1.0f;
    m_Color         = Vector4(1.0);
}

bool DirectLight::castShadows() const {
    return m_Shadows;
}

void DirectLight::setCastShadows(bool shadows) {
    m_Shadows   = shadows;
}

double DirectLight::brightness() const {
    return m_Brightness;
}

void DirectLight::setBrightness(const double brightness) {
    m_Brightness= brightness;
}

Vector4 DirectLight::color() const {
    return m_Color;
}
void DirectLight::setColor(const Vector4 &color) {
    m_Color = color;
}
