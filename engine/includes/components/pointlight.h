#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "baselight.h"

class PointLightPrivate;

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
    ~PointLight() override;

    float attenuationRadius() const;
    void setAttenuationRadius(float radius);

    float sourceRadius() const;
    void setSourceRadius(float radius);

    float sourceLength() const;
    void setSourceLength(float length);

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, PipelineContext *context, RenderList &components) override;

    AABBox bound() const override;

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    PointLightPrivate *p_ptr;

};

#endif // POINTLIGHT_H
