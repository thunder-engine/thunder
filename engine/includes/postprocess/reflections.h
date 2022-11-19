#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "pipelinepass.h"

class Texture;
class MaterialInstance;
class RenderTarget;

class Reflections : public PipelinePass {
public:
    Reflections();
    ~Reflections();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    uint32_t layer() const override;

private:
    MaterialInstance *m_iblMaterial;
    MaterialInstance *m_material;

    Texture *m_resultTexture;
    Texture *m_environmentTexture;
    Texture *m_sslrTexture;

    RenderTarget *m_resultTarget;
    RenderTarget *m_sslrTarget;

};

#endif // REFLECTIONS_H
