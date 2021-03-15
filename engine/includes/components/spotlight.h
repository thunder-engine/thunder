#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "baselight.h"

class SpotLightPrivate;

class NEXT_LIBRARY_EXPORT SpotLight : public BaseLight {
    A_REGISTER(SpotLight, BaseLight, Components)

    A_PROPERTIES(
        A_PROPERTY(float, attenuationDistance, SpotLight::attenuationDistance, SpotLight::setAttenuationDistance),
        A_PROPERTY(float, outerAngle,          SpotLight::outerAngle, SpotLight::setOuterAngle)
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
    void draw(ICommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, Pipeline *pipeline, ObjectList &components) override;

    AABBox bound() const override;
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    SpotLightPrivate *p_ptr;

};

#endif // SPOTLIGHT_H
