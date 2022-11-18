#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "renderpass.h"

#include <amath.h>

#define KERNEL_SIZE 16

class RenderTarget;
class MaterialInstance;

class AmbientOcclusion : public RenderPass {
public:
    AmbientOcclusion(PipelineContext *context);
    ~AmbientOcclusion();

    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    uint32_t layer() const override;

    const char *name() const override;

protected:
    Vector3 m_samplesKernel[KERNEL_SIZE];

    float m_radius;
    float m_bias;
    float m_power;

    Texture *m_noiseTexture;
    Texture *m_ssaoTexture;
    Texture *m_resultTexture;

    RenderTarget *m_ssaoTarget;
    RenderTarget *m_blurTarget;
    RenderTarget *m_resultTarget;

    MaterialInstance *m_blur;
    MaterialInstance *m_combine;
    MaterialInstance *m_material;

};

#endif // AMBIENTOCCLUSION_H
