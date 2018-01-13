#ifndef ABLURGL_H
#define ABLURGL_H

#include "resources/atexturegl.h"
#include "resources/amaterialgl.h"

#define MAX_SAMPLES     32

class Engine;
class APipeline;

class ABlurGL {
public:
    ABlurGL                     (APipeline *pipeline);

    void                        draw                (ATextureGL &source, ATextureGL &target, ATextureGL &temp, Vector2 &size, uint8_t steps, float *points);

protected:
    APipeline                  *m_pPipeline;

    AMaterialGL                *m_pBlurMaterial;

    int                         u_Steps;
    int                         u_Size;
    int                         u_Curve;
    int                         u_Direction;
};

#endif // ABLURGL_H
