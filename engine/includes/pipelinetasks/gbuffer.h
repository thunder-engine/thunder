#ifndef GBUFFER_H
#define GBUFFER_H

#include "pipelinetask.h"

class RenderTarget;

class GBuffer : public PipelineTask {
    A_REGISTER(GBuffer, PipelineTask, Pipeline)

public:
    GBuffer();

private:
    void exec() override;

private:
    RenderTarget *m_gbuffer;

};

#endif // GBUFFER_H
