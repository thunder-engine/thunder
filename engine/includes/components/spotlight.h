#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "baselight.h"

class SpotLightPrivate;

class NEXT_LIBRARY_EXPORT SpotLight : public BaseLight {
    A_REGISTER(SpotLight, BaseLight, Components)

    A_PROPERTIES(
        A_PROPERTY(float,   Attenuation_Distance, SpotLight::distance, SpotLight::setDistance),
        A_PROPERTY(float,   Outer_Angle,          SpotLight::angle, SpotLight::setAngle)
    )

public:
    SpotLight ();
    ~SpotLight ();

    float distance () const;
    void setDistance (float value);

    float angle () const;
    void setAngle (float value);

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, Pipeline *pipeline, ObjectList &components) override;

    AABBox bound() const override;
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    SpotLightPrivate *p_ptr;

};

#endif // SPOTLIGHT_H
