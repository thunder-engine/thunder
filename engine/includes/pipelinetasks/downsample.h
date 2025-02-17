#ifndef DOWNSAMPLE_H
#define DOWNSAMPLE_H

#include <amath.h>

#include "pipelinetask.h"

#define NUMBER_OF_PASSES 5

class RenderTarget;
class MaterialInstance;

class Downsample : public PipelineTask {
    A_REGISTER(Downsample, PipelineTask, Pipeline)

    struct DownPass {
        MaterialInstance *downMaterial = nullptr;

        RenderTarget *downTarget = nullptr;

        Texture *downTexture = nullptr;

    };

public:
    Downsample();
    ~Downsample();

private:
    void exec(PipelineContext &context) override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *source) override;

private:
    DownPass m_downPasses[NUMBER_OF_PASSES];

};

#endif // DOWNSAMPLE_H
