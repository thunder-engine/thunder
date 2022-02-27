#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "baselight.h"

class DirectLightPrivate;

class Camera;

class ENGINE_EXPORT DirectLight : public BaseLight {
    A_REGISTER(DirectLight, BaseLight, Components/Lights)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    DirectLight();
    ~DirectLight();

private:
    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, Pipeline *pipeline, RenderList &components) override;

    AABBox bound() const override;

#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    DirectLightPrivate *p_ptr;

};

#endif // DIRECTLIGHT_H
