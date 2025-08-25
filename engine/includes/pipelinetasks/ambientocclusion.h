#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "pipelinetask.h"

#include <amath.h>

class RenderTarget;
class MaterialInstance;

class AmbientOcclusion : public PipelineTask {
    A_OBJECT(AmbientOcclusion, PipelineTask, Pipeline)

public:
    AmbientOcclusion();
    ~AmbientOcclusion();

private:
    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *texture) override;

    void setContext(PipelineContext *context) override;

protected:
    float m_radius;
    float m_bias;
    float m_power;

    Texture *m_noiseTexture;
    Texture *m_aoTexture;
    Texture *m_blurTexture;

    RenderTarget *m_aoTarget;
    RenderTarget *m_blurTarget;

    MaterialInstance *m_occlusion;
    MaterialInstance *m_blur;

};

#endif // AMBIENTOCCLUSION_H
