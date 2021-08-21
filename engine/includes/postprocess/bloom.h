#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "postprocessor.h"

#include "filters/blur.h"

#define BLOOM_PASSES 5

class Engine;

class Bloom : public PostProcessor {
    struct BloomPass {
        Vector3 m_blurSize;

        float m_blurPoints[MAX_SAMPLES];

        Texture *m_downTexture;

        int32_t m_blurSteps;
    };

public:
    Bloom();

private:
    Texture *draw(Texture *source, Pipeline *pipeline) override;

    void resize(int32_t width, int32_t height) override;

    void setSettings(const PostProcessSettings &settings) override;

    const char *name() const override;

private:
    float m_threshold;

    int32_t m_width;
    int32_t m_height;

    BloomPass m_bloomPasses[BLOOM_PASSES];

};

#endif // BLOOM_H

