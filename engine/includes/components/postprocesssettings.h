#ifndef POSTPROCESSSETTINGS_H
#define POSTPROCESSSETTINGS_H

#include "component.h"

class NEXT_LIBRARY_EXPORT PostProcessSettings : public Component {
    A_REGISTER(PostProcessSettings, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(float, Ambient_Light/Intensity, PostProcessSettings::ambientLightIntensity, PostProcessSettings::setAmbientLightIntensity),
        A_PROPERTY(float, Ambient_Occlusion/Radius, PostProcessSettings::ambientOcclusionRadius, PostProcessSettings::setAmbientOcclusionRadius),
        A_PROPERTY(float, Ambient_Occlusion/Bias, PostProcessSettings::ambientOcclusionBias, PostProcessSettings::setAmbientOcclusionBias),
        A_PROPERTY(float, Ambient_Occlusion/Power, PostProcessSettings::ambientOcclusionPower, PostProcessSettings::setAmbientOcclusionPower),
        A_PROPERTY(float, Bloom/Threshold, PostProcessSettings::bloomThreshold, PostProcessSettings::setBloomThreshold)
    )
    A_NOMETHODS()

public:
    PostProcessSettings();

    float ambientLightIntensity() const;
    void setAmbientLightIntensity(float value);

    float ambientOcclusionRadius() const;
    void setAmbientOcclusionRadius(float value);

    float ambientOcclusionBias() const;
    void setAmbientOcclusionBias(float value);

    float ambientOcclusionPower() const;
    void setAmbientOcclusionPower(float value);

    float bloomThreshold() const;
    void setBloomThreshold(float value);

private:
    float m_ambientLightIntensity;

    float m_ambientOcclusionRadius;
    float m_ambientOcclusionBias;
    float m_ambientOcclusionPower;

    float m_bloomThreshold;
};

#endif // POSTPROCESSSETTINGS_H
