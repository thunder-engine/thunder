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
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    Texture *m_resultTexture;

    RenderTarget *m_resultTarget;

    MaterialInstance *m_dofMaterial;

    float m_focusDistance;

    float m_focusScale;

    float m_blurSize;

    float m_skyDistance;

};

#endif // DEPTHOFFIELD_H
