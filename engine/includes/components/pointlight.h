#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "baselight.h"

class ENGINE_EXPORT PointLight : public BaseLight {
    A_REGISTER(PointLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationRadius, PointLight::attenuationRadius, PointLight::setAttenuationRadius),
        A_PROPERTY(float, sourceRadius,     PointLight::sourceRadius, PointLight::setSourceRadius),
        A_PROPERTY(float, sourceLength,     PointLight::sourceLength, PointLight::setSourceLength)
    )
    A_NOMETHODS()

public:
    PointLight();

    float attenuationRadius() const;
    void setAttenuationRadius(float radius);

    float sourceRadius() const;
    void setSourceRadius(float radius);

    float sourceLength() const;
    void setSourceLength(float length);

private:
    int lightType() const override;

    AABBox bound() const override;

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    AABBox m_box;

};

#endif // POINTLIGHT_H
