#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include "renderpass.h"

class RenderTarget;
class MaterialInstance;

class AntiAliasing : public RenderPass {
public:
    AntiAliasing(PipelineContext *context);

    Texture *draw(Texture *source, PipelineContext *context) override;

    uint32_t layer() const override;

    void resize(int32_t width, int32_t height) override;

    const char *name() const override;

private:
    Texture *m_resultTexture;

    RenderTarget *m_resultTarget;

    MaterialInstance *m_material;

};

#endif // ANTIALIASING_H
