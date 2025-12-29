#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "pipelinecontext.h"
#include "pipelinetask.h"

class CommandBuffer;
class RenderTarget;
class MaterialInstance;
class AtlasNode;

class Camera;

class AreaLight;
class DirectLight;
class SpotLight;
class PointLight;

class ShadowMap : public PipelineTask {
    A_OBJECT(ShadowMap, PipelineTask, Pipeline)

public:
    ShadowMap();

private:
    void exec() override;

    void areaLightUpdate(AreaLight *light, const RenderList &components);
    void directLightUpdate(DirectLight *light, const RenderList &components);
    void pointLightUpdate(PointLight *light, const RenderList &components);
    void spotLightUpdate(SpotLight *light, const RenderList &components);

    void cleanShadowCache();

    RenderTarget *requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count);

private:
    struct AtlasData {
        std::vector<AtlasNode *> nodes;

        RenderTarget *target = nullptr;

        AtlasNode *sub = nullptr;

        bool unused = true;
    };

    std::unordered_map<uint32_t, AtlasData> m_tiles;
    std::unordered_map<RenderTarget *, AtlasNode *> m_shadowPages;

    std::vector<Quaternion> m_directions;

    Matrix4 m_scale;

    float m_bias;

    uint32_t m_shadowResolution;

};

#endif // SHADOWMAP_H
