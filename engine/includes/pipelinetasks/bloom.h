#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "pipelinetask.h"

#define MAX_SAMPLES 32

class RenderTarget;
class MaterialInstance;

class Bloom : public PipelineTask {
    A_OBJECT(Bloom, PipelineTask, Pipeline)

    struct BloomPass {
        float blurPoints[MAX_SAMPLES];

        int32_t steps;
    };

public:
    Bloom();
    ~Bloom();

private:
    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *source) override;

    void generateKernel(float radius, int32_t steps, float *points);

private:
    std::vector<BloomPass> m_bloomPasses;

    RenderTarget *m_resultTarget = nullptr;

    RenderTarget *m_blurTempTarget = nullptr;

    RenderTarget *m_downTarget = nullptr;

    Texture *m_downTexture = nullptr;

    Texture *m_blurTempTexture = nullptr;

    MaterialInstance *m_thresholdMaterial = nullptr;

    MaterialInstance *m_downMaterial = nullptr;

    MaterialInstance *m_blurMaterial = nullptr;

    uint32_t m_mipLevels;

};

#endif // BLOOM_H

