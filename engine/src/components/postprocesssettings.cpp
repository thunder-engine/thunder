#include "components/postprocesssettings.h"

PostProcessSettings::PostProcessSettings() :
        m_ambientLightEnabled(false),
        m_ambientLightIntensity(0.3f),
        m_ambientOcclusionEnabled(false),
        m_ambientOcclusionRadius(0.1f),
        m_ambientOcclusionBias(0.0f),
        m_ambientOcclusionPower(1.0f),
        m_bloomEnabled(false),
        m_bloomThreshold(1.0f),
        m_reflectionsEnabled(false),
        m_reflectionsMaxRoughness(0.8f) {

}

bool PostProcessSettings::ambientLightEnabled() const {
    return m_ambientLightEnabled;
}

void PostProcessSettings::setAmbientLightEnabled(bool value) {
    m_ambientLightEnabled = value;
}

float PostProcessSettings::ambientLightIntensity() const {
    return m_ambientLightIntensity;
}

void PostProcessSettings::setAmbientLightIntensity(float value) {
    m_ambientLightIntensity = value;
}

bool PostProcessSettings::ambientOcclusionEnabled() const {
    return m_ambientOcclusionEnabled;
}

void PostProcessSettings::setAmbientOcclusionEnabled(bool value) {
    m_ambientOcclusionEnabled = value;
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

bool PostProcessSettings::bloomEnabled() const {
    return m_bloomEnabled;
}

void PostProcessSettings::setBloomEnabled(bool value) {
    m_bloomEnabled = value;
}

float PostProcessSettings::bloomThreshold() const {
    return m_bloomThreshold;
}

void PostProcessSettings::setBloomThreshold(float value) {
    m_bloomThreshold = value;
}

bool PostProcessSettings::reflectionsEnabled() const {
    return m_reflectionsEnabled;
}

void PostProcessSettings::setReflectionsEnabled(bool value) {
    m_reflectionsEnabled = value;
}

float PostProcessSettings::reflectionsMaxRoughness() const {
    return m_reflectionsMaxRoughness;
}

void PostProcessSettings::setReflectionsMaxRoughness(float value) {
    m_reflectionsMaxRoughness = value;
}
