#include "components/tilemaprender.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/tilemap.h"
#include "resources/tileset.h"
#include "resources/sprite.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"

#include "mesh.h"

namespace {
const char *gTileMap = "TileMap";
const char *gMaterial = "Material";
const char *gOverride = "texture0";
}

/*!
    \class TileMapRender
    \brief The tile map renderer is used to render the tile map.
    \inmodule Engine

    TileMapRender is a class designed for rendering tile maps within Thunder Engine.
    It manages the rendering of a tile map, including handling materials, layers, and transformations.
*/

TileMapRender::TileMapRender() :
        m_tileMap(nullptr),
        m_layer(0) {

}
/*!
    \internal
*/
void TileMapRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_tileMap && !m_materials.empty() && layer & a->layers() && a->transform()) {
        buffer.setObjectId(a->uuid());
        buffer.setMaterialId(material()->uuid());
        buffer.setColor(Vector4(1.0f));

        buffer.drawMesh(a->transform()->worldTransform(), m_tileMap->tileMesh(), 0, layer, m_materials.front());
    }
}
/*!
    \internal
*/
AABBox TileMapRender::localBound() const {
    if(m_tileMap) {
        return m_tileMap->tileMesh()->bound();
    }
    return Renderable::localBound();
}
/*!
    Returns a pointer to the TileMap associated with this TileMapRender.
*/
TileMap *TileMapRender::tileMap() const {
    return m_tileMap;
}
/*!
    Sets the TileMap associated with this TileMapRender.
*/
void TileMapRender::setTileMap(TileMap *map) {
    m_tileMap = map;

    if(m_tileMap) {
        TileSet *tileSet = m_tileMap->tileSet();
        if(tileSet) {
            Texture *texture = tileSet->spriteSheet() ? tileSet->spriteSheet()->texture() : nullptr;
            if(texture && !m_materials.empty()) {
                m_materials.front()->setTexture(gOverride, texture);
            }
        }
    }
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void TileMapRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    if(m_tileMap) {
        TileSet *tileSet = m_tileMap->tileSet();
        if(tileSet) {
            Texture *texture = tileSet->spriteSheet() ? tileSet->spriteSheet()->texture() : nullptr;
            if(texture && !m_materials.empty()) {
                m_materials.front()->setTexture(gOverride, texture);
            }
        }
    }

}
/*!
    Returns the order layer for the tile map.
*/
int TileMapRender::layer() const {
    return m_layer;
}
/*!
    Sets the order \a layer for the tile map.
*/
void TileMapRender::setLayer(int layer) {
    m_layer = layer;
}
/*!
    \internal
*/
void TileMapRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gMaterial);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(gTileMap);
        if(it != data.end()) {
            setTileMap(Engine::loadResource<TileMap>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap TileMapRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[gMaterial] = ref;
        }
    }
    {
        string ref = Engine::reference(tileMap());
        if(!ref.empty()) {
            result[gTileMap] = ref;
        }
    }

    return result;
}
/*!
    \internal
*/
void TileMapRender::composeComponent() {
    setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
}
/*!
    \internal
*/
int TileMapRender::priority() const {
    return m_layer;
}
