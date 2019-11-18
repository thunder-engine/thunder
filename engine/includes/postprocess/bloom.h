#ifndef BLOOM_H
#define BLOOM_H

#include <amath.h>

#include "postprocessor.h"

#include "filters/blur.h"

#define BLOOM_PASSES 5

class Engine;

class Bloom : public PostProcessor {
    struct BloomPass {
        RenderTexture *m_pDownTexture;

        float m_BlurPoints[MAX_SAMPLES];

        int32_t m_BlurSteps;

        Vector3 m_BlurSize;
    };

public:
    Bloom ();

    RenderTexture *draw (RenderTexture *source, ICommandBuffer &buffer);

    void resize (int32_t width, int32_t height);

private:
    float m_Threshold;

    BloomPass m_BloomPasses[BLOOM_PASSES];

};

#endif // BLOOM_H

