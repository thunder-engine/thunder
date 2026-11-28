/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "components/tilemaprender.h"

#include "resources/tilemap.h"
#include "resources/tileset.h"
#include "resources/mesh.h"

namespace {
    const char *gTileMap("TileMap");
    const char *gMaterial("Material");
    const char *gColor("mainColor");
    const char *gTexture("mainTexture");
    const char *gDefaultSprite(".engine/DefaultSprite.shader");
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

Mesh *TileMapRender::meshToDraw() {
    return m_tileMap ? m_tileMap->tileMesh() : nullptr;
}
/*!
    \internal
*/
AABBox TileMapRender::localBound() {
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
                Texture *texture = tileSet->texture();
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
            Texture *texture = tileSet->texture();
            Vector4 color(1.0f);
            for(auto it : m_materials) {
                it->setTexture(gTexture, texture);
                it->setVector4(gColor, &color);
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
    }
}
/*!
    \internal
*/
void TileMapRender::composeComponent() {
    setMaterial(Engine::loadResource<Material>(gDefaultSprite));
}
