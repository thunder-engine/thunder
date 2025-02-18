#ifndef DEFERREDLIGHTING_H
#define DEFERREDLIGHTING_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class DeferredLighting : public PipelineTask {
    A_REGISTER(DeferredLighting, PipelineTask, Pipeline)

public:
    DeferredLighting();
    ~DeferredLighting();

private:
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    Vector3 m_sunDirection;

    RenderTarget *m_lightPass;

};

#endif // DEFERREDLIGHTING_H
