#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "baselight.h"

class SpotLightPrivate;

class ENGINE_EXPORT SpotLight : public BaseLight {
    A_REGISTER(SpotLight, BaseLight, Components/Lights)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationDistance, SpotLight::attenuationDistance, SpotLight::setAttenuationDistance),
        A_PROPERTY(float, outerAngle, SpotLight::outerAngle, SpotLight::setOuterAngle)
    )
    A_NOMETHODS()

public:
    SpotLight();
    ~SpotLight();

    float attenuationDistance() const;
    void setAttenuationDistance(float distance);

    float outerAngle() const;
    void setOuterAngle(float value);

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, Pipeline *pipeline, RenderList &components) override;

    AABBox bound() const override;
#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    SpotLightPrivate *p_ptr;

};

#endif // SPOTLIGHT_H
