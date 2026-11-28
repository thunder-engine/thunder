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

#ifndef TILEMAPRENDER_H
#define TILEMAPRENDER_H

#include <renderable.h>

#include <tilemap.h>
#include <material.h>

class Mesh;
class MaterialInstance;

class ENGINE_EXPORT TileMapRender : public Renderable {
    A_OBJECT(TileMapRender, Renderable, Components/2D)

    A_PROPERTIES(
        A_PROPERTYEX(TileMap *, tileMap, TileMapRender::tileMap, TileMapRender::setTileMap, "editor=Asset"),
        A_PROPERTYEX(Material *, material, TileMapRender::material, TileMapRender::setMaterial, "editor=Asset"),
        A_PROPERTY(int, layer, TileMapRender::layer, TileMapRender::setLayer)
    )
    A_NOMETHODS()

public:
    TileMapRender();
    ~TileMapRender();

    TileMap *tileMap() const;
    void setTileMap(TileMap *map);

    void setMaterial(Material *material) override;

    int layer() const;
    void setLayer(int layer);

private:
    AABBox localBound() override;

    Mesh *meshToDraw() override;

    void setMaterialsList(const std::list<Material *> &materials) override;

    void composeComponent() override;

private:
    TileMap *m_tileMap;

    int m_priority;
};

#endif // TILEMAPRENDER_H
