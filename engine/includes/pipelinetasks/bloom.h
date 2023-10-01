#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "pipelinetask.h"

#include "filters/blur.h"

#define BLOOM_PASSES 5

class RenderTarget;

class Bloom : public PipelineTask {
    A_REGISTER(Bloom, PipelineTask, Pipeline)

    struct BloomPass {
        Vector3 m_blurSize;

        float m_blurPoints[MAX_SAMPLES];

        Texture *m_downTexture;

        int32_t m_blurSteps;
    };

public:
    Bloom();

private:
    void exec(PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    void setInput(int index, Texture *source) override;

private:
    BloomPass m_bloomPasses[BLOOM_PASSES];

    MaterialInstance *m_material;

    RenderTarget *m_resultTarget;

    float m_threshold;

};

#endif // BLOOM_H

