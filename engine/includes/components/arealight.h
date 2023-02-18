#ifndef AREALIGHT_H
#define AREALIGHT_H

#include "baselight.h"

class ENGINE_EXPORT AreaLight : public BaseLight {
    A_REGISTER(AreaLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationRadius, AreaLight::radius, AreaLight::setRadius),
        A_PROPERTY(float, sourceWidth,       AreaLight::sourceWidth, AreaLight::setSourceWidth),
        A_PROPERTY(float, sourceHeight,      AreaLight::sourceHeight, AreaLight::setSourceHeight)
    )
    A_NOMETHODS()

public:
    AreaLight();

    float radius() const;
    void setRadius(float radius);

    float sourceWidth() const;
    void setSourceWidth(float width);

    float sourceHeight() const;
    void setSourceHeight(float height);

private:
    int lightType() const override;

    AABBox bound() const override;

    void drawGizmos() override;
    void drawGizmosSelected() override;

private:
    AABBox m_box;

};

#endif // AREALIGHT_H
