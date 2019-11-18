#include "components/postprocesssettings.h"

PostProcessSettings::PostProcessSettings() :
        m_ambientLightIntensity(0.3f),
        m_ambientOcclusionRadius(0.1f),
        m_ambientOcclusionBias(0.0f),
        m_ambientOcclusionPower(1.0f),
        m_bloomThreshold(1.0f) {

}

float PostProcessSettings::ambientLightIntensity() const {
    return m_ambientLightIntensity;
}

void PostProcessSettings::setAmbientLightIntensity(float value) {
    m_ambientLightIntensity = value;
}

float PostProcessSettings::ambientOcclusionRadius() const {
    return m_ambientOcclusionRadius;
}

void PostProcessSettings::setAmbientOcclusionRadius(float value) {
    m_ambientOcclusionRadius = value;
}

float PostProcessSettings::ambientOcclusionBias() const {
    return m_ambientOcclusionBias;
}

void PostProcessSettings::setAmbientOcclusionBias(float value) {
    m_ambientOcclusionBias = value;
}

float PostProcessSettings::ambientOcclusionPower() const {
    return m_ambientOcclusionPower;
}

void PostProcessSettings::setAmbientOcclusionPower(float value) {
    m_ambientOcclusionPower = value;
}

float PostProcessSettings::bloomThreshold() const {
    return m_bloomThreshold;
}

void PostProcessSettings::setBloomThreshold(float value) {
    m_bloomThreshold = value;
}
