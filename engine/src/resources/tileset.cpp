#include "resources/tileset.h"

#include "resources/sprite.h"

#include <variant.h>

#define SPRITE_SHEET "SpriteSheet"

const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;

/*!
    \class TileSet
    \brief A TileSet is a collection of tiles that can be placed in a TileMap.
    \inmodule Resources
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

int TileSet::type() const {
    return m_type;
}

void TileSet::setType(int type) {
    m_type = type;
}

int TileSet::tileWidth() const {
    return m_tileWidth;
}

void TileSet::setTileWidth(int width) {
    m_tileWidth = width;
}

int TileSet::tileHeight() const {
    return m_tileHeight;
}

void TileSet::setTileHeight(int height) {
    m_tileHeight = height;
}

int TileSet::tileSpacing() const {
    return m_tileSpacing;
}

void TileSet::setTileSpacing(int spacing) {
    m_tileSpacing = spacing;
}

int TileSet::tileMargin() const {
    return m_tileMargin;
}

void TileSet::setTileMargin(int margin) {
    m_tileMargin = margin;
}

Sprite *TileSet::spriteSheet() const {
    return m_spriteSheet;
}

void TileSet::setSpriteSheet(Sprite *sprite) {
    m_spriteSheet = sprite;
    if(m_spriteSheet && m_spriteSheet->texture()) {
        m_width = m_spriteSheet->texture()->width();
        m_height = m_spriteSheet->texture()->height();

        if(m_columns == 0) {
            m_columns = m_width / m_tileWidth;
        }
    } else {
        m_width = 0;
        m_height = 0;
        m_columns = 0;
    }
}

int TileSet::columns() const {
    return m_columns;
}

void TileSet::setColumns(int columns) {
    m_columns = columns;
}

Vector2 TileSet::tileOffset() const {
    return m_tileOffset;
}

void TileSet::setTileOffset(const Vector2 offset) {
    m_tileOffset = offset;
}

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
    auto it = data.find(SPRITE_SHEET);
    if(it != data.end()) {
        setSpriteSheet(Engine::loadResource<Sprite>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap TileSet::saveUserData() const {
    VariantMap result;

    string ref = Engine::reference(spriteSheet());
    if(!ref.empty()) {
        result[SPRITE_SHEET] = ref;
    }

    return result;
}
