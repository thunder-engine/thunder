#ifndef TRANSLUCENT_H
#define TRANSLUCENT_H

#include "pipelinetask.h"

class RenderTarget;

class Translucent : public PipelineTask {
    A_REGISTER(Translucent, PipelineTask, Pipeline)

public:
    Translucent();

private:
    void exec(PipelineContext *context) override;

    void setInput(int index, Texture *texture) override;

private:
    RenderTarget *m_translucentPass;

};

#endif // TRANSLUCENT_H
