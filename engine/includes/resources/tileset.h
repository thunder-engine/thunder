#ifndef TILESET_H
#define TILESET_H

#include <sprite.h>

class ENGINE_EXPORT TileSet : public Resource {
    A_REGISTER(TileSet, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTY(TileType, type, TileSet::type, TileSet::setType),
        A_PROPERTY(int, tileWidth, TileSet::tileWidth, TileSet::setTileWidth),
        A_PROPERTY(int, tileHeight, TileSet::tileHeight, TileSet::setTileHeight),
        A_PROPERTY(int, tileSpacing, TileSet::tileSpacing, TileSet::setTileSpacing),
        A_PROPERTY(int, tileMargin, TileSet::tileMargin, TileSet::setTileMargin),
        A_PROPERTY(int, columns, TileSet::columns, TileSet::setColumns),
        A_PROPERTY(Vector2, tileOffset, TileSet::tileOffset, TileSet::setTileOffset),
        A_PROPERTYEX(Sprite *, spriteSheet, TileSet::spriteSheet, TileSet::setSpriteSheet, "editor=Asset")
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

    Sprite *spriteSheet() const;
    void setSpriteSheet(Sprite *sprite);

    int columns() const;
    void setColumns(int columns);

    Vector2 tileOffset() const;
    void setTileOffset(const Vector2 offset);

    Vector4 getCorners(int index);

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    int m_type;

    int m_tileWidth;

    int m_tileHeight;

    int m_tileSpacing;

    int m_tileMargin;

    int m_columns;

    int m_width;

    int m_height;

    Vector2 m_tileOffset;

    Sprite *m_spriteSheet;

};

#endif // TILESET_H
