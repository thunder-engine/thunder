#ifndef BLUR_H
#define BLUR_H

#include <amath.h>

class Mesh;
class RenderTexture;
class MaterialInstance;

class ICommandBuffer;

#define MAX_SAMPLES 32

class Blur {
public:
    Blur ();

    void draw (ICommandBuffer &buffer, RenderTexture *source, RenderTexture *target);

    void setParameters(const Vector2 &size, int32_t steps, float *points);

protected:
    MaterialInstance *m_pBlurMaterial;

    Mesh *m_pMesh;

    int32_t m_Steps;
    float m_Points[MAX_SAMPLES];

    RenderTexture *m_Temp;

    Vector2 m_Size;
    Vector2 m_Direction;
};

#endif // BLUR_H
