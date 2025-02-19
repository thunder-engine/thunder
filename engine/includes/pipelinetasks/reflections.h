#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class Reflections : public PipelineTask {
    A_REGISTER(Reflections, PipelineTask, Pipeline)

public:
    Reflections();

private:
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    MaterialInstance *m_sslrMaterial;

    Texture *m_sslrTexture;

    RenderTarget *m_sslrTarget;

};

#endif // REFLECTIONS_H
