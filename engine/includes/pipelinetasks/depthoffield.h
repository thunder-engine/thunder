#ifndef DEPTHOFFIELD_H
#define DEPTHOFFIELD_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class DepthOfField : public PipelineTask {
    A_OBJECT(DepthOfField, PipelineTask, Pipeline)

public:
    DepthOfField();
    ~DepthOfField();

private:
    void analyze(World *world) override;

    void exec() override;

    void resize(int32_t width, int32_t height) override;

    void setInput(int index, Texture *texture) override;

    void generateKernel(float radius, int32_t steps, float *points);

private:
    Texture *m_downTexture = nullptr;

    Texture *m_blurTexture = nullptr;

    Texture *m_resultTexture = nullptr;

    RenderTarget *m_downTarget = nullptr;

    RenderTarget *m_blurTarget = nullptr;

    RenderTarget *m_resultTarget = nullptr;

    MaterialInstance *m_downMaterial;

    MaterialInstance *m_blurMaterial;

    MaterialInstance *m_dofMaterial;

    float m_focusDistance;

    float m_focusScale;

    float m_blurSize;

    float m_skyDistance;

};

#endif // DEPTHOFFIELD_H
