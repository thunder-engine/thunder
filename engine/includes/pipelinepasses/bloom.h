#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "pipelinepass.h"

#include "filters/blur.h"

#define BLOOM_PASSES 5

class RenderTarget;

class Bloom : public PipelinePass {
    struct BloomPass {
        Vector3 m_blurSize;

        float m_blurPoints[MAX_SAMPLES];

        Texture *m_downTexture;

        int32_t m_blurSteps;
    };

public:
    Bloom();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

private:
    float m_threshold;

    int32_t m_width;
    int32_t m_height;

    BloomPass m_bloomPasses[BLOOM_PASSES];

    MaterialInstance *m_material;

    RenderTarget *m_resultTarget;

};

#endif // BLOOM_H

