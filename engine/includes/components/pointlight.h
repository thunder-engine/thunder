#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "baselight.h"

class PointLightPrivate;

class NEXT_LIBRARY_EXPORT PointLight : public BaseLight {
    A_REGISTER(PointLight, BaseLight, Components)

    A_PROPERTIES(
        A_PROPERTY(float,   Attenuation_Radius,   PointLight::radius, PointLight::setRadius)
    )

public:
    PointLight ();
    ~PointLight ();

    float radius () const;
    void setRadius (float radius);

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

#ifdef NEXT_SHARED
    bool drawHandles() override;
#endif

private:
    PointLightPrivate *p_ptr;

};

#endif // POINTLIGHT_H
