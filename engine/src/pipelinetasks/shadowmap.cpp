#include "pipelinetasks/shadowmap.h"

#include "engine.h"

#include "commandbuffer.h"

#include "components/baselight.h"

#include "utils/atlas.h"
#include "resources/rendertarget.h"
#include "resources/material.h"

namespace {
    const char *gShadowmap("g.shadowmap");

    const char *gShadowMap("shadowMap");
    const char *gUniTiles("tiles");
    const char *gUniShadows("shadows");
};

ShadowMap::ShadowMap() :
        m_root(new AtlasNode),
        m_shadowTarget(Engine::objectCreate<RenderTarget>()),
        m_shadowMap(Engine::objectCreate<Texture>("shadowAtlas")),
        m_shadowAtlasSize(MIN(8192, Texture::maxTextureSize())),
        m_shadowTileSize(2048) {

    setName("ShadowMap");

    Engine::setValue(gShadowmap, true);

    m_shadowMap->setFormat(Texture::Depth);
    m_shadowMap->setDepthBits(24);
    m_shadowMap->setFlags(Texture::Render);
    m_shadowMap->resize(m_shadowAtlasSize, m_shadowAtlasSize);

    m_shadowTarget->setDepthAttachment(m_shadowMap);
    m_shadowTarget->setFlags(RenderTarget::ClearDepth | RenderTarget::Atlas);

    m_root->w = m_shadowAtlasSize;
    m_root->h = m_shadowAtlasSize;

    m_outputs.push_back(std::make_pair(m_shadowMap->name(), m_shadowMap));
}

void ShadowMap::analyze(World *world) {
    A_UNUSED(world);
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

    buffer->setRenderTarget(m_shadowTarget);
    for(auto &it : m_context->sceneLights()) {
        auto instance = it->material();
        if(instance) {
            float shadows = it->castShadows() ? 1.0f : 0.0f;
            instance->setFloat(gUniShadows, &shadows);
        }

        if(it->castShadows()) {
            lightUpdate(it, it->tilesCount());
        }
    }
    m_shadowTarget->setTileIndex(-1);

    m_context->cameraReset();
    buffer->endDebugMarker();
}

void ShadowMap::lightUpdate(BaseLight *light, int count) {
    const auto t = requestShadowTiles(light->uuid(), 0, count);
    if(t) {
        const std::vector<AtlasNode *> &nodes(*t);
        Vector4 tiles[6];

        CommandBuffer *buffer = m_context->buffer();
        for(int32_t i = count - 1; i >= 0; i--) {
            tiles[i] = Vector4(static_cast<float>(nodes[i]->x) / m_shadowAtlasSize,
                               static_cast<float>(nodes[i]->y) / m_shadowAtlasSize,
                               static_cast<float>(nodes[i]->w) / m_shadowAtlasSize,
                               static_cast<float>(nodes[i]->h) / m_shadowAtlasSize);

            const Renderable::GroupList &groups = light->groups(i);
            if(!groups.empty()) {
                uint32_t index = nodes[i]->x / m_shadowTileSize + (nodes[i]->y / m_shadowTileSize) * (m_shadowAtlasSize / m_shadowTileSize);
                m_shadowTarget->setTileIndex(index);

                buffer->setViewProjection(light->cropMatrix(i));
                buffer->setViewport(nodes[i]->x, nodes[i]->y, nodes[i]->w, nodes[i]->h);

                // Draw to the depth buffer from the position of the light source
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
            instance->setVector4(gUniTiles, tiles);
            instance->setTexture(gShadowMap, m_shadowMap);
        }
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

const std::vector<AtlasNode *> *ShadowMap::requestShadowTiles(uint32_t id, uint32_t lod, uint32_t count) {
    auto tile = m_tiles.find(id);
    if(tile != m_tiles.end()) {
        tile->second.unused = false;
        return &(tile->second.nodes);
    }

    int32_t width = (m_shadowTileSize >> lod);
    int32_t height = (m_shadowTileSize >> lod);

    uint32_t columns = MAX(count / 2, 1);
    uint32_t rows = count / columns;

    AtlasNode *sub = m_root->insert(width * columns, height * rows);
    if(sub) {
        std::vector<AtlasNode *> tiles;
        tiles.reserve(count);
        for(uint32_t i = 0; i < count; i++) {
            AtlasNode *node = sub->insert(width, height);
            if(node) {
                node->occupied = true;
                tiles.push_back(node);
            }
        }
        if(tiles.size() == count) {
            m_tiles[id] = {tiles, sub, false};
            return &(m_tiles[id].nodes);
        }

        m_shadowTarget->setRenderArea(tiles[0]->x, tiles[0]->y, width * columns, height * rows);
    }
    return nullptr;
}

void ShadowMap::resize(int width, int height) {
    if(m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
    }
}
