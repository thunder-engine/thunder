#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "pipelinepass.h"

#include <amath.h>

class RenderTarget;
class MaterialInstance;

class AmbientOcclusion : public PipelinePass {
public:
    enum Inputs {
        Input
    };
public:
    AmbientOcclusion();
    ~AmbientOcclusion();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    uint32_t layer() const override;

    void setInput(uint32_t index, Texture *texture) override;

    uint32_t outputCount() const override;
    Texture *output(uint32_t index) override;

protected:
    float m_radius;
    float m_bias;
    float m_power;

    Texture *m_noiseTexture;
    Texture *m_ssaoTexture;
    Texture *m_blurTexture;

    RenderTarget *m_ssaoTarget;
    RenderTarget *m_blurTarget;
    RenderTarget *m_combineTarget;

    MaterialInstance *m_occlusion;
    MaterialInstance *m_blur;
    MaterialInstance *m_combine;

};

#endif // AMBIENTOCCLUSION_H
