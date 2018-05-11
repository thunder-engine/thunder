#ifndef ABLURGL_H
#define ABLURGL_H

#include "resources/atexturegl.h"
#include "resources/amaterialgl.h"

#define MAX_SAMPLES     32

class Engine;
class APipeline;

class ABlurGL {
public:
    ABlurGL                     ();

    void                        draw                (ICommandBuffer &buffer, ATextureGL &source, ATextureGL &target, ATextureGL &temp, Vector2 &size, uint8_t steps, float *points);

protected:
    AMaterialGL                *m_pBlurMaterial;

    int                         u_Steps;
    int                         u_Size;
    int                         u_Curve;
    int                         u_Direction;
};

#endif // ABLURGL_H
