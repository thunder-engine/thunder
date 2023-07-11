#ifndef GBUFFER_H
#define GBUFFER_H

#include "pipelinepass.h"

#define DEPTH_MAP   "depthMap"
#define G_EMISSIVE  "emissiveMap"
#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"

#define VISIBILITY_BUFFER "visibilityBuffer"

class RenderTarget;

class GBuffer : public PipelinePass {
public:
    GBuffer();

    enum Outputs {
        Emissive,
        Normals,
        Diffuse,
        Params,
        Depth,
        OutputsNumber // Do not add anything after this
    };

private:
    Texture *draw(Texture *source, PipelineContext *context) override;

    void resize(int32_t width, int32_t height) override;

    uint32_t layer() const override;

    uint32_t outputCount() const override;
    Texture *output(uint32_t index) override;

    void enableVisibility();

private:
    RenderTarget *m_gbuffer;

    Texture *m_depth;
    Texture *m_emissive;
    Texture *m_normals;
    Texture *m_diffuse;
    Texture *m_params;

    Texture *m_radiance;

    int32_t m_width;
    int32_t m_height;

};

#endif // GBUFFER_H
