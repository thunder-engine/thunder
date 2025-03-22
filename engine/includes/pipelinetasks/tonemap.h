#ifndef TONEMAP_H
#define TONEMAP_H

#include "pipelinetask.h"

class RenderTarget;
class MaterialInstance;

class Tonemap : public PipelineTask {
    A_OBJECT(Tonemap, PipelineTask, Pipeline)

public:
    Tonemap();
    ~Tonemap();

private:
    void exec() override;

    void setInput(int index, Texture *texture) override;

private:
    Texture *m_resultTexture;
    Texture *m_defaultLutTexture;
    Texture *m_lutTexture;

    RenderTarget *m_resultTarget;

    MaterialInstance *m_resultMaterial;

};

#endif // TONEMAP_H
