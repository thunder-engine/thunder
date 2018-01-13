#include "components/lightSource.h"

LightSource::LightSource() {
    m_Type          = DIRECT;
    m_Shadows       = false;
    m_Brightness    = 1.0f;
    m_Radius        = 1.0f;
}

bool LightSource::castShadows() const {
    return m_Shadows;
}

void LightSource::setCastShadows(bool shadows) {
    m_Shadows   = shadows;
}

double LightSource::brightness() const {
    return m_Brightness;
}

void LightSource::setBrightness(const double brightness) {
    m_Brightness= brightness;
}

double LightSource::radius() const {
    return m_Radius;
}

void LightSource::setRadius(const double radius) {
    m_Radius    = radius;
}
