#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "pipelinetask.h"

#define BLOOM_PASSES 5

class RenderTarget;
class MaterialInstance;

class Bloom : public PipelineTask {
    A_REGISTER(Bloom, PipelineTask, Pipeline)

    struct BloomPass {
        MaterialInstance *downMaterial = nullptr;

        MaterialInstance *blurMaterialH = nullptr;

        MaterialInstance *blurMaterialV = nullptr;

        RenderTarget *downTarget = nullptr;

        Texture *downTexture = nullptr;

        float blurSize;

    };

public:
    Bloom();

private:
    void exec(PipelineContext &context) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    void setInput(int index, Texture *source) override;

    void generateKernel(float radius, int32_t steps, float *points);

private:
    BloomPass m_bloomPasses[BLOOM_PASSES];

    RenderTarget *m_resultTarget = nullptr;

    RenderTarget *m_blurTempTarget = nullptr;

    Texture *m_blurTempTexture = nullptr;

    float m_threshold;

};

#endif // BLOOM_H

