#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "pipelinetask.h"

#include <amath.h>

class RenderTarget;
class MaterialInstance;

class AmbientOcclusion : public PipelineTask {
    A_REGISTER(AmbientOcclusion, PipelineTask, Pipeline)

public:
    AmbientOcclusion();
    ~AmbientOcclusion();

private:
    void exec(PipelineContext &context) override;

    void setSettings(const PostProcessSettings &settings) override;

    void setInput(int index, Texture *texture) override;

protected:
    float m_radius;
    float m_bias;
    float m_power;

    Texture *m_noiseTexture;

    RenderTarget *m_ssaoTarget;
    RenderTarget *m_blurTarget;
    RenderTarget *m_combineTarget;

    MaterialInstance *m_occlusion;
    MaterialInstance *m_blur;
    MaterialInstance *m_combine;

};

#endif // AMBIENTOCCLUSION_H
