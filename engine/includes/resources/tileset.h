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

#ifndef TILESET_H
#define TILESET_H

#include <sprite.h>

class ENGINE_EXPORT TileSet : public Resource {
    A_OBJECT(TileSet, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(TileType, type, TileSet::type, TileSet::setType),
        A_PROPERTY(int, tileWidth, TileSet::tileWidth, TileSet::setTileWidth),
        A_PROPERTY(int, tileHeight, TileSet::tileHeight, TileSet::setTileHeight),
        A_PROPERTY(int, tileSpacing, TileSet::tileSpacing, TileSet::setTileSpacing),
        A_PROPERTY(int, tileMargin, TileSet::tileMargin, TileSet::setTileMargin),
        A_PROPERTY(int, columns, TileSet::columns, TileSet::setColumns),
        A_PROPERTY(Vector2, tileOffset, TileSet::tileOffset, TileSet::setTileOffset),
        A_PROPERTYEX(Texture *, texture, TileSet::texture, TileSet::setTexture, "editor=Asset")
    )
    A_ENUMS(
        A_ENUM(TileType,
               A_VALUE(Orthogonal),
               A_VALUE(Isometric))
    )

public:
    enum TileType {
        Orthogonal = 0,
        Isometric,
        Hexagonal
    };

public:
    TileSet();

    int type() const;
    void setType(int type);

    int tileWidth() const;
    void setTileWidth(int width);

    int tileHeight() const;
    void setTileHeight(int height);

    int tileSpacing() const;
    void setTileSpacing(int spacing);

    int tileMargin() const;
    void setTileMargin(int margin);

    Texture *texture() const;
    void setTexture(Texture *texture);

    int columns() const;
    void setColumns(int columns);

    Vector2 tileOffset() const;
    void setTileOffset(const Vector2 offset);

    Vector4 getCorners(int index);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    Vector2 m_tileOffset;

    Texture *m_texture;

    int m_type;

    int m_tileWidth;

    int m_tileHeight;

    int m_tileSpacing;

    int m_tileMargin;

    int m_columns;

    int m_width;

    int m_height;

};

#endif // TILESET_H
