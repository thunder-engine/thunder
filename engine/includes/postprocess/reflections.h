#ifndef REFLECTIONS_H
#define REFLECTIONS_H

#include "renderpass.h"

class Texture;
class MaterialInstance;
class RenderTarget;

class Reflections : public RenderPass {
public:
    Reflections(PipelineContext *context);
    ~Reflections();

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    uint32_t layer() const override;

    const char *name() const override;

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
