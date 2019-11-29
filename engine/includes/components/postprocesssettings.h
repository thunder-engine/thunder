#ifndef POSTPROCESSSETTINGS_H
#define POSTPROCESSSETTINGS_H

#include "component.h"

class NEXT_LIBRARY_EXPORT PostProcessSettings : public Component {
    A_REGISTER(PostProcessSettings, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(bool,  Ambient_Light/Enabled, PostProcessSettings::ambientLightEnabled, PostProcessSettings::setAmbientLightEnabled),
        A_PROPERTY(float, Ambient_Light/Intensity, PostProcessSettings::ambientLightIntensity, PostProcessSettings::setAmbientLightIntensity),
        A_PROPERTY(bool,  Ambient_Occlusion/Enabled, PostProcessSettings::ambientOcclusionEnabled, PostProcessSettings::setAmbientOcclusionEnabled),
        A_PROPERTY(float, Ambient_Occlusion/Radius, PostProcessSettings::ambientOcclusionRadius, PostProcessSettings::setAmbientOcclusionRadius),
        A_PROPERTY(float, Ambient_Occlusion/Bias, PostProcessSettings::ambientOcclusionBias, PostProcessSettings::setAmbientOcclusionBias),
        A_PROPERTY(float, Ambient_Occlusion/Power, PostProcessSettings::ambientOcclusionPower, PostProcessSettings::setAmbientOcclusionPower),
        A_PROPERTY(bool, Bloom/Enabled, PostProcessSettings::bloomEnabled, PostProcessSettings::setBloomEnabled),
        A_PROPERTY(float, Bloom/Threshold, PostProcessSettings::bloomThreshold, PostProcessSettings::setBloomThreshold)
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

private:
    bool m_ambientLightEnabled;
    float m_ambientLightIntensity;

    bool m_ambientOcclusionEnabled;
    float m_ambientOcclusionRadius;
    float m_ambientOcclusionBias;
    float m_ambientOcclusionPower;

    bool m_bloomEnabled;
    float m_bloomThreshold;
};

#endif // POSTPROCESSSETTINGS_H
