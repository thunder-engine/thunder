#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "postprocessor.h"

#include <amath.h>

#define KERNEL_SIZE 16

class Texture;

class AmbientOcclusion : public PostProcessor {
public:
    AmbientOcclusion ();

    ~AmbientOcclusion () override;

    Texture *draw(Texture *source, Pipeline *pipeline) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    uint32_t layer() const override;

    const char *name() const override;

protected:
    Vector3 m_samplesKernel[KERNEL_SIZE];
    float m_blurSamplesKernel[4];

    float m_radius;
    float m_bias;
    float m_power;

    Texture *m_noiseTexture;
    Texture *m_ssaoTexture;

    RenderTarget *m_ssaoTarget;
    RenderTarget *m_blurTarget;

    MaterialInstance *m_blur;
    MaterialInstance *m_occlusion;
};

#endif // AMBIENTOCCLUSION_H
