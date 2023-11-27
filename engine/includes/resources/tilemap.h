#ifndef TILEMAP_H
#define TILEMAP_H

#include <resource.h>

#include <tileset.h>

class Mesh;

class ENGINE_EXPORT TileMap : public Resource {
    A_REGISTER(TileMap, Resource, Resources)

    A_PROPERTIES(
        A_PROPERTYEX(TileSet *, tileSet, TileMap::tileSet, TileMap::setTileSet, "editor=Asset"),
        A_PROPERTY(int, width, TileMap::width, TileMap::setWidth),
        A_PROPERTY(int, height, TileMap::height, TileMap::setHeight),
        A_PROPERTY(int, orientation, TileMap::orientation, TileMap::setOrientation),
        A_PROPERTY(int, cellWidth, TileMap::cellWidth, TileMap::setCellWidth),
        A_PROPERTY(int, cellHeight, TileMap::cellHeight, TileMap::setCellHeight),
        A_PROPERTY(bool, hexOdd, TileMap::hexOdd, TileMap::setHexOdd),
        A_PROPERTY(bool, hexSideLength, TileMap::hexSideLength, TileMap::setHexSideLength)
    )

public:
    TileMap();

    TileSet *tileSet() const;
    void setTileSet(TileSet *set);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    int tile(int x, int y) const;
    void setTile(int x, int y, int id);

    int orientation() const;
    void setOrientation(int orientation);

    int cellWidth() const;
    void setCellWidth(int width);

    int cellHeight() const;
    void setCellHeight(int height);

    bool hexOdd() const;
    void setHexOdd(bool odd);

    int hexSideLength() const;
    void setHexSideLength(int length);

    Mesh *tileMesh() const;

    void refreshAllTiles() const;

protected:
    void loadUserData(const VariantMap &data) override;

    VariantMap saveUserData() const override;

    Vector4 getCorners(int index, int tileWidth, int tileHeight, int width, int height, int columns, int spacing, int margin);

private:
    std::vector<int> m_data;

    int m_width;

    int m_height;

    int m_cellWidth;

    int m_cellHeight;

    int m_orientation;

    int m_staggerIndex;

    int m_hexSizeLength;

    TileSet *m_tileSet;

    Mesh *m_tileMesh;

    mutable bool m_dirty;

};

#endif // TILEMAP_H
