#include "resources/tileset.h"

#include "resources/sprite.h"

#include <variant.h>

namespace  {
    const char *gSpriteSheet = "SpriteSheet";
}

const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;

/*!
    \class TileSet
    \brief A TileSet is a collection of tiles that can be placed in a TileMap.
    \inmodule Resources

    TileSet is a resource class used to define and manage collections of individual tiles.
    These tiles are typically used in conjunction with a TileMap to create complex game layouts.
*/

TileSet::TileSet() :
        m_type(0),
        m_tileWidth(0),
        m_tileHeight(0),
        m_tileSpacing(1),
        m_tileMargin(0),
        m_columns(0),
        m_width(0),
        m_height(0),
        m_spriteSheet(nullptr) {

}
/*!
    Returns the type of the tileset.
    This can represent the orientation or layout style of the tiles.
*/
int TileSet::type() const {
    return m_type;
}
/*!
     Sets the \a type of the tileset, specifying the orientation or layout style of the tiles.
*/
void TileSet::setType(int type) {
    m_type = type;
}
/*!
    Returns the width of an individual tile in pixels.
*/
int TileSet::tileWidth() const {
    return m_tileWidth;
}
/*!
    Sets the \a width of an individual tile in pixels.
*/
void TileSet::setTileWidth(int width) {
    m_tileWidth = width;
}
/*!
    Returns the height of an individual tile in pixels.
*/
int TileSet::tileHeight() const {
    return m_tileHeight;
}
/*!
    Sets the \a height of an individual tile in pixels.
*/
void TileSet::setTileHeight(int height) {
    m_tileHeight = height;
}
/*!
    Returns the spacing (gap) between adjacent tiles in pixels.
*/
int TileSet::tileSpacing() const {
    return m_tileSpacing;
}
/*!
    Sets the \a spacing (gap) between adjacent tiles in pixels.
*/
void TileSet::setTileSpacing(int spacing) {
    m_tileSpacing = spacing;
}
/*!
    Returns the margin (border) around the tiles in pixels.
*/
int TileSet::tileMargin() const {
    return m_tileMargin;
}
/*!
    Sets the \a margin (border) around the tiles in pixels.
*/
void TileSet::setTileMargin(int margin) {
    m_tileMargin = margin;
}
/*!
    Returns a pointer to the sprite sheet containing the individual tiles.
*/
Sprite *TileSet::spriteSheet() const {
    return m_spriteSheet;
}
/*!
    Sets the sprite \a sheet containing the individual tiles.
*/
void TileSet::setSpriteSheet(Sprite *sheet) {
    m_spriteSheet = sheet;
    if(m_spriteSheet) {
        Texture *t = m_spriteSheet->page();
        if(t) {
            m_width = t->width();
            m_height = t->height();

            if(m_columns == 0) {
                m_columns = m_width / m_tileWidth;
            }
        }
    } else {
        m_width = 0;
        m_height = 0;
        m_columns = 0;
    }
}
/*!
    Returns the number of columns in the tileset.
*/
int TileSet::columns() const {
    return m_columns;
}
/*!
    Sets the number of \a columns in the tileset.
*/
void TileSet::setColumns(int columns) {
    m_columns = columns;
}
/*!
     Returns the offset used for tile positioning.
*/
Vector2 TileSet::tileOffset() const {
    return m_tileOffset;
}
/*!
    Sets the \a offset used for tile positioning.
*/
void TileSet::setTileOffset(const Vector2 offset) {
    m_tileOffset = offset;
}
/*!
    Calculates and returns the texture coordinates (corners) of a specific tile within the tileset based on its \a index.
    This method considers tile flipping (horizontal and vertical) if applicable.
*/
Vector4 TileSet::getCorners(int index) {
    Vector4 result;

    bool flippedHorizontally = (index & FLIPPED_HORIZONTALLY_FLAG);
    bool flippedVertically = (index & FLIPPED_VERTICALLY_FLAG);

    index &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG);

    int x = index % m_columns;
    int y = index / m_columns;

    float modX = m_tileWidth + (x * m_tileSpacing) + m_tileMargin;
    float modY = m_tileHeight + (y * m_tileSpacing) + m_tileMargin;

    result.x = ((float)(x + (flippedHorizontally ? 1 : 0)) * modX) / (float)m_width;
    result.z = ((float)(x + (flippedHorizontally ? 0 : 1)) * modX) / (float)m_width;

    result.w = 1.0f - ((float)(y + (flippedVertically ? 1 : 0)) * modY) / (float)m_height;
    result.y = 1.0f - ((float)(y + (flippedVertically ? 0 : 1)) * modY) / (float)m_height;

    return result;
}
/*!
    \internal
*/
void TileSet::loadUserData(const VariantMap &data) {
    auto it = data.find(gSpriteSheet);
    if(it != data.end()) {
        setSpriteSheet(Engine::loadResource<Sprite>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap TileSet::saveUserData() const {
    VariantMap result;

    String ref = Engine::reference(spriteSheet());
    if(!ref.isEmpty()) {
        result[gSpriteSheet] = ref;
    }

    return result;
}
