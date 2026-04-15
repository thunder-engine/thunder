#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "baselight.h"

class SpotLightPrivate;

class ENGINE_EXPORT SpotLight : public BaseLight {
    A_OBJECT(SpotLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationDistance, SpotLight::attenuationDistance, SpotLight::setAttenuationDistance),
        A_PROPERTY(float, outerAngle, SpotLight::outerAngle, SpotLight::setOuterAngle)
    )
    A_NOMETHODS()

public:
    SpotLight();

    float attenuationDistance() const;
    void setAttenuationDistance(float distance);

    float outerAngle() const;
    void setOuterAngle(float value);

private:
    void cleanDirty() override;

    int lightType() const override;

    bool isCulled(const Frustum &frustum, const Matrix4 &viewProjection) override;

    int tilesCount() const override;

    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    float m_angle;

};

#endif // SPOTLIGHT_H
