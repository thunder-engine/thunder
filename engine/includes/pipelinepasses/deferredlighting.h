#ifndef DEFERREDLIGHTING_H
#define DEFERREDLIGHTING_H

#include "pipelinepass.h"

class RenderTarget;

class DeferredLighting : public PipelinePass {

public:
    enum Inputs {
        Emissve,
        Depth
    };

public:
    DeferredLighting();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    uint32_t layer() const override;

    void setInput(uint32_t index, Texture *texture) override;

private:
    RenderTarget *m_lightPass;

};

#endif // DEFERREDLIGHTING_H
