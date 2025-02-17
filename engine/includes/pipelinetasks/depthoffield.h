#ifndef DEPTHOFFIELD_H
#define DEPTHOFFIELD_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class DepthOfField : public PipelineTask {
    A_REGISTER(DepthOfField, PipelineTask, Pipeline)

public:
    DepthOfField();
    ~DepthOfField();

private:
    void exec(PipelineContext &context) override;

    void setInput(int index, Texture *texture) override;

private:
    Texture *m_resultTexture;

    RenderTarget *m_resultTarget;

    MaterialInstance *m_dofMaterial;

};

#endif // DEPTHOFFIELD_H
