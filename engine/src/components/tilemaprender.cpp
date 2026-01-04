#include "components/tilemaprender.h"

#include "resources/tilemap.h"
#include "resources/tileset.h"
#include "resources/sprite.h"
#include "resources/mesh.h"

namespace {
    const char *gTileMap("TileMap");
    const char *gMaterial("Material");
    const char *gColor("mainColor");
    const char *gTexture("mainTexture");
    const char *gDefaultSprite(".embedded/DefaultSprite.shader");
}

/*!
    \class TileMapRender
    \brief The tile map renderer is used to render the tile map.
    \inmodule Components

    TileMapRender is a class designed for rendering tile maps within Thunder Engine.
    It manages the rendering of a tile map, including handling materials, layers, and transformations.
*/

TileMapRender::TileMapRender() :
        m_tileMap(nullptr),
        m_priority(0) {

}

TileMapRender::~TileMapRender() {
    if(m_tileMap) {
        m_tileMap->decRef();
    }
}

Mesh *TileMapRender::meshToDraw(int instance) {
    A_UNUSED(instance);
    return m_tileMap ? m_tileMap->tileMesh() : nullptr;
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
    Returns a pointer to the tile map associated with this TileMapRender.
*/
TileMap *TileMapRender::tileMap() const {
    return m_tileMap;
}
/*!
    \fn void TileMapRender::setTileMap(TileMap *map)

    Sets the tile \a map associated with this TileMapRender.
*/
void TileMapRender::setTileMap(TileMap *map) {
    if(m_tileMap != map) {
        if(m_tileMap) {
            m_tileMap->decRef();
        }

        m_tileMap = map;
        if(m_tileMap) {
            m_tileMap->incRef();

            TileSet *tileSet = m_tileMap->tileSet();
            if(tileSet) {
                Texture *texture = tileSet->spriteSheet() ? tileSet->spriteSheet()->page() : nullptr;
                Vector4 color(1.0f);
                for(auto it : m_materials) {
                    it->setTexture(gTexture, texture);
                    it->setVector4(gColor, &color);
                }
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
            Texture *texture = nullptr;
            Sprite *sheet = tileSet->spriteSheet();
            if(sheet) {
                texture = sheet->page();
            }
            Vector4 color(1.0f);
            for(auto it : m_materials) {
                it->setTexture(gTexture, texture);
                it->setVector4(gColor, &color);
                it->setTransform(transform());
            }
        }
    }

}
/*!
    Returns the redering priority for the tile map.
*/
int TileMapRender::layer() const {
    return m_priority;
}
/*!
    Sets the redering \a layer for the tile map.
*/
void TileMapRender::setLayer(int layer) {
    m_priority = layer;

    for(auto it : m_materials) {
        it->setPriority(m_priority);
    }
}
/*!
    \internal
*/
void TileMapRender::setMaterialsList(const std::list<Material *> &materials) {
    Renderable::setMaterialsList(materials);

    Vector4 color(1.0f);
    for(auto it : m_materials) {
        it->setVector4(gColor, &color);
        it->setTransform(transform());
    }
}
/*!
    \internal
*/
void TileMapRender::composeComponent() {
    setMaterial(Engine::loadResource<Material>(gDefaultSprite));
}
