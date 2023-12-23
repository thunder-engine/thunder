#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "pipelinetask.h"

class Texture;
class MaterialInstance;
class RenderTarget;

class Reflections : public PipelineTask {
    A_REGISTER(Reflections, PipelineTask, Pipeline)

public:
    Reflections();

private:
    void exec(PipelineContext &context) override;

    void setInput(int index, Texture *texture) override;

private:
    MaterialInstance *m_slrMaterial;
    MaterialInstance *m_combineMaterial;

    Texture *m_environmentTexture;

    RenderTarget *m_slrTarget;
    RenderTarget *m_combineTarget;

};

#endif // REFLECTIONS_H
