#ifndef GBUFFER_H
#define GBUFFER_H

#include "pipelinetask.h"

class RenderTarget;

class GBuffer : public PipelineTask {
    A_OBJECT(GBuffer, PipelineTask, Pipeline)

public:
    GBuffer();

private:
    void exec() override;

    void analyze(World *world) override;

private:
    std::list<Group> m_opaque;

    RenderTarget *m_gbuffer;

};

#endif // GBUFFER_H
