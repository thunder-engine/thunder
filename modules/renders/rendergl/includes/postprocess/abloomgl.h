#ifndef ABLOOMGL
#define ABLOOMGL

#include <amath.h>

#include "postprocess/apostprocessor.h"

#include "filters/ablurgl.h"

#define BLOOM_PASSES    5

class Engine;

class ABloomGL : public APostProcessor {
    struct BloomPass {
        ATextureGL              DownTexture;

        float                   BlurPoints[MAX_SAMPLES];

        uint32_t                BlurSteps;

        Vector3                 BlurSize;
    };

public:
    ABloomGL                    ();

    RenderTexture              *draw                (RenderTexture &source, ICommandBuffer &buffer);

    /*!
        Resizing current texture buffers.
        @param[in]  width       New screen width.
        @param[in]  height      New screen height.
    */
    void                        resize              (int32_t width, int32_t height);

private:
    float                       m_Threshold;

    BloomPass                   m_BloomPasses[BLOOM_PASSES];

    ATextureGL                  m_BlurTemp;
};

#endif // ABLOOMGL

