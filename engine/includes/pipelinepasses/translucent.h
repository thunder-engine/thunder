#ifndef TRANSLUCENT_H
#define TRANSLUCENT_H

#include "pipelinepass.h"

class RenderTarget;

class Translucent : public PipelinePass {

public:
    enum Inputs {
        Emissve,
        Depth
    };

public:
    Translucent();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    uint32_t layer() const override;

    void setInput(uint32_t index, Texture *texture) override;

private:
    RenderTarget *m_translucentPass;

};

#endif // TRANSLUCENT_H
