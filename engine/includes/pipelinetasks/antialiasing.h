#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class AntiAliasing : public PipelineTask {
    A_REGISTER(AntiAliasing, PipelineTask, Pipeline)

public:
    AntiAliasing();
    ~AntiAliasing();

private:
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    Texture *m_resultTexture;

    RenderTarget *m_resultTarget;

    MaterialInstance *m_resultMaterial;

};

#endif // ANTIALIASING_H
