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

    RenderTexture *draw(RenderTexture *source, Pipeline *pipeline) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    uint32_t layer() const override;

protected:
    Vector3 m_SamplesKernel[KERNEL_SIZE];
    float m_BlurSamplesKernel[4];

    float m_Radius;
    float m_Bias;
    float m_Power;

    Texture *m_pNoise;
    RenderTexture *m_pSSAO;

    MaterialInstance *m_pBlur;
    MaterialInstance *m_pOcclusion;
};

#endif // AMBIENTOCCLUSION_H
