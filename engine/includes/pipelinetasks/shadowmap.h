#ifndef SHADOWMAP_H
#define SHADOWMAP_H

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
class Renderable;

class ShadowMap : public PipelineTask {
    A_REGISTER(ShadowMap, PipelineTask, Pipeline)

public:
    ShadowMap();

private:
    void exec() override;

    void areaLightUpdate(AreaLight *light, std::list<Renderable *> &components);
    void directLightUpdate(DirectLight *light, std::list<Renderable *> &components);
    void pointLightUpdate(PointLight *light, std::list<Renderable *> &components);
    void spotLightUpdate(SpotLight *light, std::list<Renderable *> &components);

    void cleanShadowCache();

    RenderTarget *requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count);

private:
    std::unordered_map<uint32_t, std::pair<RenderTarget *, std::vector<AtlasNode *>>> m_tiles;
    std::unordered_map<RenderTarget *, AtlasNode *> m_shadowPages;

    std::vector<Quaternion> m_directions;

    Matrix4 m_scale;

    float m_bias;

    uint32_t m_shadowResolution;

};

#endif // SHADOWMAP_H
