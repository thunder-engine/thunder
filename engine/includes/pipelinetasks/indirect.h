#ifndef DEFERREDINDIRECT_H
#define DEFERREDINDIRECT_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class DeferredIndirect : public PipelineTask {
    A_REGISTER(DeferredIndirect, PipelineTask, Pipeline)

public:
    DeferredIndirect();

private:
    void analyze(World *world) override;
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    Vector4 m_cameraColor;

    MaterialInstance *m_iblMaterial;

    Texture *m_cameraTexture;
    Texture *m_iblTexture;

    RenderTarget *m_iblTarget;

};

#endif // DEFERREDINDIRECT_H
