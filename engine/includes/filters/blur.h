#ifndef BLUR_H
#define BLUR_H

#include <amath.h>

class Mesh;
class Texture;
class RenderTarget;
class MaterialInstance;

class CommandBuffer;

#define MAX_SAMPLES 32

class Blur {
public:
    Blur();

    void draw(CommandBuffer &buffer, Texture *source, RenderTarget *target);

    void setParameters(const Vector2 &size, int32_t steps, const float *points);

    static void generateKernel(float radius, int32_t steps, float *points);

protected:
    Vector2 m_direction;

    MaterialInstance *m_blurMaterial;

    Mesh *m_mesh;

    Texture *m_tempTexture;
    RenderTarget *m_tempTarget;

};

#endif // BLUR_H
