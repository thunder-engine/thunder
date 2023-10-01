#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class AntiAliasing : public PipelineTask {
    A_REGISTER(AntiAliasing, PipelineTask, Pipeline)

public:
    AntiAliasing();

private:
    void exec(PipelineContext *context) override;

    void setInput(int index, Texture *texture) override;

private:
    RenderTarget *m_resultTarget;

    MaterialInstance *m_material;

};

#endif // ANTIALIASING_H
