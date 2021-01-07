#ifndef POSTPROCESSSETTINGS_H
#define POSTPROCESSSETTINGS_H

#include "component.h"

class NEXT_LIBRARY_EXPORT PostProcessSettings : public Component {
    A_REGISTER(PostProcessSettings, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(bool,  ambientLight/Enabled, PostProcessSettings::ambientLightEnabled, PostProcessSettings::setAmbientLightEnabled),
        A_PROPERTY(float, ambientLight/Intensity, PostProcessSettings::ambientLightIntensity, PostProcessSettings::setAmbientLightIntensity),
        A_PROPERTY(bool,  ambientOcclusion/Enabled, PostProcessSettings::ambientOcclusionEnabled, PostProcessSettings::setAmbientOcclusionEnabled),
        A_PROPERTY(float, ambientOcclusion/Radius, PostProcessSettings::ambientOcclusionRadius, PostProcessSettings::setAmbientOcclusionRadius),
        A_PROPERTY(float, ambientOcclusion/Bias, PostProcessSettings::ambientOcclusionBias, PostProcessSettings::setAmbientOcclusionBias),
        A_PROPERTY(float, ambientOcclusion/Power, PostProcessSettings::ambientOcclusionPower, PostProcessSettings::setAmbientOcclusionPower),
        A_PROPERTY(bool, bloom/Enabled, PostProcessSettings::bloomEnabled, PostProcessSettings::setBloomEnabled),
        A_PROPERTY(float, bloom/Threshold, PostProcessSettings::bloomThreshold, PostProcessSettings::setBloomThreshold),
        A_PROPERTY(bool, reflections/Enabled, PostProcessSettings::reflectionsEnabled, PostProcessSettings::setReflectionsEnabled),
        A_PROPERTY(float, reflections/MaxRoughness, PostProcessSettings::reflectionsMaxRoughness, PostProcessSettings::setReflectionsMaxRoughness)
    )
    A_NOMETHODS()

public:
    PostProcessSettings();

    bool ambientLightEnabled() const;
    void setAmbientLightEnabled(bool value);

    float ambientLightIntensity() const;
    void setAmbientLightIntensity(float value);


    bool ambientOcclusionEnabled() const;
    void setAmbientOcclusionEnabled(bool value);

    float ambientOcclusionRadius() const;
    void setAmbientOcclusionRadius(float value);

    float ambientOcclusionBias() const;
    void setAmbientOcclusionBias(float value);

    float ambientOcclusionPower() const;
    void setAmbientOcclusionPower(float value);


    bool bloomEnabled() const;
    void setBloomEnabled(bool value);

    float bloomThreshold() const;
    void setBloomThreshold(float value);


    bool reflectionsEnabled() const;
    void setReflectionsEnabled(bool value);

    float reflectionsMaxRoughness() const;
    void setReflectionsMaxRoughness(float value);

private:
    bool m_ambientLightEnabled;
    float m_ambientLightIntensity;

    bool m_ambientOcclusionEnabled;
    float m_ambientOcclusionRadius;
    float m_ambientOcclusionBias;
    float m_ambientOcclusionPower;

    bool m_bloomEnabled;
    float m_bloomThreshold;

    bool m_reflectionsEnabled;
    float m_reflectionsMaxRoughness;
};

#endif // POSTPROCESSSETTINGS_H
