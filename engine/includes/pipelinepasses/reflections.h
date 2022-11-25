#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "pipelinepass.h"

class Texture;
class MaterialInstance;
class RenderTarget;

class Reflections : public PipelinePass {
public:
    Reflections();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    uint32_t layer() const override;

    uint32_t outputCount() const override;

    Texture *output(uint32_t index) override;

private:
    MaterialInstance *m_slrMaterial;
    MaterialInstance *m_iblMaterial;

    Texture *m_slrTexture;
    Texture *m_environmentTexture;
    Texture *m_resultTexture;

    RenderTarget *m_slrTarget;
    RenderTarget *m_resultTarget;

};

#endif // REFLECTIONS_H
