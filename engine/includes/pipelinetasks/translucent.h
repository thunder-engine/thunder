#ifndef TRANSLUCENT_H
#define TRANSLUCENT_H

#include "pipelinetask.h"

class RenderTarget;

class Translucent : public PipelineTask {
    A_OBJECT(Translucent, PipelineTask, Pipeline)

public:
    Translucent();

private:
    void exec() override;

    void analyze(World *world) override;

    void setInput(int index, Texture *texture) override;

private:
    std::list<Group> m_translucent;

    RenderTarget *m_translucentPass;

};

#endif // TRANSLUCENT_H
