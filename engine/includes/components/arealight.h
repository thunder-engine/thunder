#ifndef AREALIGHT_H
#define AREALIGHT_H

#include "baselight.h"

class AreaLightPrivate;

class NEXT_LIBRARY_EXPORT AreaLight : public BaseLight {
    A_REGISTER(AreaLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationRadius, AreaLight::radius, AreaLight::setRadius),
        A_PROPERTY(float, sourceWidth,       AreaLight::sourceWidth, AreaLight::setSourceWidth),
        A_PROPERTY(float, sourceHeight,      AreaLight::sourceHeight, AreaLight::setSourceHeight)
    )
    A_NOMETHODS()

public:
    AreaLight();
    ~AreaLight() override;

    float radius() const;
    void setRadius(float radius);

    float sourceWidth() const;
    void setSourceWidth(float width);

    float sourceHeight() const;
    void setSourceHeight(float height);

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, Pipeline *pipeline, RenderList &components) override;

    AABBox bound() const override;

#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    AreaLightPrivate *p_ptr;

};

#endif // AREALIGHT_H
