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
        RenderTarget *blurTempTarget = nullptr;

        RenderTarget *downTarget = nullptr;

        Texture *downTexture = nullptr;

        Texture *blurTempTexture = nullptr;

        float blurPoints[MAX_SAMPLES];

        int32_t steps;
    };

public:
    Bloom();
    ~Bloom();

private:
    void analyze(World *world) override;

    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *source) override;

    void generateKernel(float radius, int32_t steps, float *points);

private:
    std::vector<BloomPass> m_bloomPasses;

    RenderTarget *m_resultTarget = nullptr;

    MaterialInstance *m_thresholdMaterial = nullptr;

    MaterialInstance *m_downMaterial = nullptr;

    MaterialInstance *m_blurMaterial = nullptr;

    Texture *m_dirtTexture = nullptr;

    uint32_t m_mipLevels;

    float m_threshold;

    float m_intensity;

    float m_dirtIntensity;

};

#endif // BLOOM_H

