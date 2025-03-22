#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "pipelinetask.h"

#define BLOOM_PASSES 5

class RenderTarget;
class MaterialInstance;

class Bloom : public PipelineTask {
    A_OBJECT(Bloom, PipelineTask, Pipeline)

    struct BloomPass {
        MaterialInstance *blurMaterialH = nullptr;

        MaterialInstance *blurMaterialV = nullptr;

        float blurSize;

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
    BloomPass m_bloomPasses[BLOOM_PASSES];

    RenderTarget *m_resultTarget = nullptr;

    RenderTarget *m_blurTempTarget = nullptr;

    Texture *m_blurTempTexture = nullptr;

    float m_threshold;

};

#endif // BLOOM_H

