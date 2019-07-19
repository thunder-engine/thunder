#ifndef DIRECTLIGHT_H
#define DIRECTLIGHT_H

#include "baselight.h"

class DirectLightPrivate;

class Camera;

class NEXT_LIBRARY_EXPORT DirectLight : public BaseLight {
    A_REGISTER(DirectLight, BaseLight, Components)

    A_NOPROPERTIES()

public:
    DirectLight ();
    ~DirectLight ();

    Vector4 &normalizedDistance ();

    Vector4 *tiles ();

    Matrix4 *matrix ();

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    void shadowsUpdate (const Camera &camera, ICommandBuffer &buffer, ObjectList &components) override;

#ifdef NEXT_SHARED
    bool drawHandles() override;
#endif

private:
    DirectLightPrivate *p_ptr;

};

#endif // DIRECTLIGHT_H
