#include "pipelinetasks/shadowmap.h"

#include "engine.h"

#include "commandbuffer.h"

#include "components/baselight.h"

#include "utils/atlas.h"
#include "resources/rendertarget.h"
#include "resources/material.h"

namespace {
    const char *gShadowmap("g.shadowmap");

    const char *shadowMap("shadowMap");
    const char *uniTiles("tiles");
    const char *uniShadows("shadows");
};

ShadowMap::ShadowMap() :
        m_shadowAtlasSize(MIN(8192, Texture::maxTextureSize())),
        m_shadowTileSize(2048) {

    setName("ShadowMap");

    Engine::setValue(gShadowmap, true);
}

void ShadowMap::analyze(World *world) {
    RenderList &components = m_context->sceneRenderables();
    for(auto &it : m_context->sceneLights()) {
        if(it->castShadows()) {
            it->buildGroups(components);
        }
    }
}

void ShadowMap::exec() {
    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("ShadowMap");
    cleanShadowCache();

    for(auto &it : m_context->sceneLights()) {
        auto instance = it->material();
        if(instance) {
            float shadows = it->castShadows() ? 1.0f : 0.0f;
            instance->setFloat(uniShadows, &shadows);
        }

        if(it->castShadows()) {
            lightUpdate(it, it->tilesCount());
        }
    }

    m_context->cameraReset();
    buffer->endDebugMarker();
}

void ShadowMap::lightUpdate(BaseLight *light, int count) {
    std::vector<Vector4> tiles;
    tiles.resize(count);

    std::vector<int32_t> x;
    x.resize(count);
    std::vector<int32_t> y;
    y.resize(count);
    std::vector<int32_t> w;
    w.resize(count);
    std::vector<int32_t> h;
    h.resize(count);

    RenderTarget *shadowTarget = requestShadowTiles(light->uuid(), 0, x.data(), y.data(), w.data(), h.data(), count);
    CommandBuffer *buffer = m_context->buffer();
    buffer->setRenderTarget(shadowTarget);
    for(int32_t i = 0; i < count; i++) {
        tiles[i] = Vector4(static_cast<float>(x[i]) / m_shadowAtlasSize,
                           static_cast<float>(y[i]) / m_shadowAtlasSize,
                           static_cast<float>(w[i]) / m_shadowAtlasSize,
                           static_cast<float>(h[i]) / m_shadowAtlasSize);

        const Renderable::GroupList &groups = light->groups(i);
        if(!groups.empty()) {
            buffer->setViewProjection(light->cropMatrix(i));
            buffer->setViewport(x[i], y[i], w[i], h[i]);

            // Draw in the depth buffer from position of the light source
            for(auto &it : groups) {
                if(it.count > 1) {
                    it.instance->setInstanceBuffer(&it.buffer);
                }
                buffer->drawMesh(it.mesh, it.subMesh, Material::Shadowcast, *it.instance);
                it.instance->setInstanceBuffer(nullptr);
            }
        }
    }

    auto instance = light->material();
    if(instance) {
        instance->setVector4(uniTiles, tiles.data(), count);
        instance->setTexture(shadowMap, shadowTarget->depthAttachment());
    }
}

void ShadowMap::cleanShadowCache() {
    for(auto tiles = m_tiles.begin(); tiles != m_tiles.end(); ) {
        if(tiles->second.unused) {
            for(auto &it : tiles->second.nodes) {
                it->occupied = false;
            }
            tiles->second.sub->clean();
            tiles = m_tiles.erase(tiles);
        } else {
            ++tiles;
        }
    }

    for(auto &tile : m_tiles) {
        tile.second.unused = true;
    }
}

RenderTarget *ShadowMap::requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count) {
    auto tile = m_tiles.find(id);
    if(tile != m_tiles.end()) {
        for(uint32_t i = 0; i < count; i++) {
            AtlasNode *node = tile->second.nodes[i];
            x[i] = node->x;
            y[i] = node->y;
            w[i] = node->w;
            h[i] = node->h;
        }
        tile->second.unused = false;
        return tile->second.target;
    }

    int32_t width = (m_shadowTileSize >> lod);
    int32_t height = (m_shadowTileSize >> lod);

    uint32_t columns = MAX(count / 2, 1);
    uint32_t rows = count / columns;

    RenderTarget *target = nullptr;
    AtlasNode *sub = nullptr;

    for(auto page : m_shadowPages) {
        target = page.first;
        AtlasNode *root = page.second;

        sub = root->insert(width * columns, height * rows);
        if(sub) {
            break;
        }
    }

    if(sub == nullptr) {
        Texture *map = Engine::objectCreate<Texture>(std::string("shadowAtlas ") + std::to_string(m_shadowPages.size()));
        map->setFormat(Texture::Depth);
        map->setDepthBits(24);
        map->setFlags(Texture::Render);

        map->resize(m_shadowAtlasSize, m_shadowAtlasSize);

        m_context->addTextureBuffer(map);

        target = Engine::objectCreate<RenderTarget>();
        target->setDepthAttachment(map);
        target->setClearFlags(RenderTarget::ClearDepth);

        AtlasNode *root = new AtlasNode;

        root->w = m_shadowAtlasSize;
        root->h = m_shadowAtlasSize;

        m_shadowPages[target] = root;

        sub = root->insert(width * columns, height * rows);
    }

    std::vector<AtlasNode *> tiles;
    for(uint32_t i = 0; i < count; i++) {
        AtlasNode *node = sub->insert(width, height);
        if(node) {
            x[i] = node->x;
            y[i] = node->y;
            w[i] = node->w;
            h[i] = node->h;
            node->occupied = true;
            tiles.push_back(node);
        }
    }
    if(tiles.size() == count) {
        m_tiles[id] = {tiles, target, sub, false};
    }

    target->setRenderArea(x[0], y[0], width * columns, height * rows);
    return target;
}
