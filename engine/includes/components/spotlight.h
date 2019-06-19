#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "baselight.h"

class SpotLightPrivate;

class NEXT_LIBRARY_EXPORT SpotLight : public BaseLight {
    A_REGISTER(SpotLight, BaseLight, Components)

    A_PROPERTIES(
        A_PROPERTY(float,   Attenuation_Radius,   SpotLight::radius, SpotLight::setRadius),
        A_PROPERTY(float,   Outer_Angle,          SpotLight::angle, SpotLight::setAngle)
    )

public:
    SpotLight ();
    ~SpotLight ();

    void draw (ICommandBuffer &buffer, uint32_t layer);

    float radius () const;
    void setRadius (float value);

    float angle () const;
    void setAngle (float value);

private:
#ifdef NEXT_SHARED
    bool drawHandles() override;
#endif

private:
    SpotLightPrivate *p_ptr;

};

#endif // SPOTLIGHT_H
