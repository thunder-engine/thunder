#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "pipelinetask.h"

class RenderTarget;
class AtlasNode;

class DirectLight;
class SpotLight;

class ShadowMap : public PipelineTask {
    A_OBJECT(ShadowMap, PipelineTask, Pipeline)

public:
    ShadowMap();

private:
    void analyze(World *world) override;
    void exec() override;

    void lightUpdate(BaseLight *light, int count);
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

    uint32_t m_shadowAtlasSize;
    uint32_t m_shadowTileSize;
};

#endif // SHADOWMAP_H
