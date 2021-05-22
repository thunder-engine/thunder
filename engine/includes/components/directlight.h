#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "baselight.h"

class DirectLightPrivate;

class Camera;

class NEXT_LIBRARY_EXPORT DirectLight : public BaseLight {
    A_REGISTER(DirectLight, BaseLight, Components/Lights)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    DirectLight();
    ~DirectLight();

private:
    void draw(ICommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate(const Camera &camera, Pipeline *pipeline, RenderList &components) override;

    AABBox bound() const override;

#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

private:
    DirectLightPrivate *p_ptr;

};

#endif // DIRECTLIGHT_H
