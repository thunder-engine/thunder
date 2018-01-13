#ifndef CAMERAGL_H
#define CAMERAGL_H

#include <components/camera.h>
#include <engine.h>

class AShaderGL;

class ACameraGL : public Camera {
    A_OVERRIDE(ACameraGL, Camera, Components)

    A_NOMETHODS()
    A_NOPROPERTIES()

public:
    /*! \enum hit_types */
    enum hit_types {
        FRUSTUM_OUTSIDE         = 0,
        FRUSTUM_INTERSECT       = 1,
        FRUSTUM_INSIDE          = 2
    };

    void                        setShaderParams     (uint32_t program);

    Vector2                   screen              () const { return Vector2(m_Width, m_Height); }

protected:

};

#endif // CAMERAGL_H
