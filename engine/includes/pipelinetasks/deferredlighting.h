#ifndef DEFERREDLIGHTING_H
#define DEFERREDLIGHTING_H

#include "pipelinetask.h"

class RenderTarget;

class DeferredLighting : public PipelineTask {
    A_REGISTER(DeferredLighting, PipelineTask, Pipeline)

public:
    DeferredLighting();

private:
    void exec(PipelineContext &context) override;

    void setInput(int index, Texture *texture) override;

private:
    RenderTarget *m_lightPass;

};

#endif // DEFERREDLIGHTING_H
