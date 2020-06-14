#ifndef AREALIGHT_H
#define AREALIGHT_H

#include "baselight.h"

class AreaLightPrivate;

class NEXT_LIBRARY_EXPORT AreaLight : public BaseLight {
    A_REGISTER(AreaLight, BaseLight, Components)

    A_PROPERTIES(
        A_PROPERTY(float, Attenuation_Radius, AreaLight::radius, AreaLight::setRadius),
        A_PROPERTY(float, Source_Width,       AreaLight::sourceWidth, AreaLight::setSourceWidth),
        A_PROPERTY(float, Source_Height,      AreaLight::sourceHeight, AreaLight::setSourceHeight)
    )

public:
    AreaLight ();
    ~AreaLight () override;

    float radius () const;
    void setRadius (float radius);

    float sourceWidth () const;
    void setSourceWidth (float width);

    float sourceHeight () const;
    void setSourceHeight (float height);

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate (const Camera &camera, Pipeline *pipeline, ObjectList &components) override;

    AABBox bound () const override;

#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    AreaLightPrivate *p_ptr;

};

#endif // AREALIGHT_H
