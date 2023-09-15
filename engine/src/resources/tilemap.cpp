#include "resources/tilemap.h"

#include "resources/tileset.h"
#include "resources/mesh.h"

#include <cstring>

#define DATA  "Data"
#define TILESET "TileSet"

/*!
    \class TileMap
    \brief A TileMap is a grid of tiles used to create a game's layout.
    \inmodule Resources
*/

TileMap::TileMap() :
        m_width(0),
        m_height(0),
        m_cellWidth(1),
        m_cellHeight(1),
        m_orientation(-1),
        m_staggerIndex(1),
        m_hexSizeLength(0),
        m_tileSet(nullptr),
        m_tileMesh(Engine::objectCreate<Mesh>()),
        m_dirty(true) {

}

TileSet *TileMap::tileSet() const {
    return m_tileSet;
}

void TileMap::setTileSet(TileSet *set) {
    m_tileSet = set;
    if(m_tileSet && m_orientation == -1) {
        m_orientation = m_tileSet->type();
    }
    m_dirty = true;
}

int TileMap::width() const {
    return m_width;
}

void TileMap::setWidth(int width) {
    if(m_width != width) {
        m_width = width;
        m_data.resize(m_width * m_height);
        m_dirty = true;
    }
}

int TileMap::height() const {
    return m_height;
}

void TileMap::setHeight(int height) {
    if(m_height != height) {
        m_height = height;
        m_data.resize(m_width * m_height);
        m_dirty = true;
    }
}

int TileMap::tile(int x, int y) const {
    return m_data[y * m_width + x];
}

void TileMap::setTile(int x, int y, int id) {
    m_data[y * m_width + x] = id;
    m_dirty = true;
}

int TileMap::orientation() const {
    return m_orientation;
}

void TileMap::setOrientation(int orientation) {
    m_orientation = orientation;
    m_dirty = true;
}

int TileMap::cellWidth() const {
    return m_cellWidth;
}

void TileMap::setCellWidth(int width) {
    m_cellWidth = width;
    m_dirty = true;
}

int TileMap::cellHeight() const {
    return m_cellHeight;
}

void TileMap::setCellHeight(int height) {
    m_cellHeight = height;
    m_dirty = true;
}

bool TileMap::hexOdd() const {
    return m_staggerIndex == 1;
}

void TileMap::setHexOdd(bool odd) {
    m_staggerIndex = odd ? 1 : 0;
    m_dirty = true;
}

int TileMap::hexSideLength() const {
    return m_hexSizeLength;
}

void TileMap::setHexSideLength(int length) {
    m_hexSizeLength = length;
    m_dirty = true;
}

Mesh *TileMap::tileMesh() const {
    if(m_dirty) {
        refreshAllTiles();

        m_dirty = false;
    }

    return m_tileMesh;
}

void TileMap::refreshAllTiles() const {
    if(m_tileSet == nullptr) {
        return;
    }

    Vector2 cellSize(m_cellWidth, m_cellHeight);
    Vector2 tileSize(m_tileSet->tileWidth(), m_tileSet->tileHeight());

    Vector2 tileOffset(m_tileSet->tileOffset());

    m_tileMesh->clear();

    Vector3Vector vertices;
    vertices.resize(m_width * m_height * 4);

    Vector2Vector uvs;
    uvs.resize(m_width * m_height * 4);

    IndexVector indices;
    indices.reserve(m_width * m_height * 6);

    Vector2 offset;
    switch(m_orientation) {
    case TileSet::Isometric: {
        offset.x = cellSize.x * (m_width-1) * 0.5f;
        offset.y = -cellSize.y;
        offset += tileOffset;
        break;
    }
    case TileSet::Hexagonal: {
        offset.x = (m_staggerIndex == 1) ? 0.0f : cellSize.x * 0.5f;
        offset.y = -cellSize.y * 0.5f;
        offset += tileOffset;
        break;
    }
    default: break;
    }

    int index = 0;
    for(int y = 0; y < m_height; y++) {
        for(int x = 0; x < m_width; x++) {
            int id = m_data[y * m_width + x];
            if(id == -1) {
                offset.x += cellSize.x;
                continue;
            }

            Vector4 uv = m_tileSet->getCorners(id);

            vertices[index].x = offset.x;
            vertices[index].y = -offset.y;

            vertices[index + 1].x = offset.x;
            vertices[index + 1].y = -offset.y - tileSize.y;

            vertices[index + 2].x = offset.x + tileSize.x;
            vertices[index + 2].y = -offset.y - tileSize.y;

            vertices[index + 3].x = offset.x + tileSize.x;
            vertices[index + 3].y = -offset.y;

            uvs[index].x = uv.x;
            uvs[index].y = uv.w;

            uvs[index + 1].x = uv.x;
            uvs[index + 1].y = uv.y;

            uvs[index + 2].x = uv.z;
            uvs[index + 2].y = uv.y;

            uvs[index + 3].x = uv.z;
            uvs[index + 3].y = uv.w;

            indices.push_back(index);
            indices.push_back(index + 1);
            indices.push_back(index + 2);
            indices.push_back(index);
            indices.push_back(index + 2);
            indices.push_back(index + 3);
            index += 4;

            if(m_orientation == TileSet::Isometric) {
                offset.x += cellSize.x * 0.5f;
                offset.y += cellSize.y * 0.5f;
            } else {
                offset.x += cellSize.x;
            }
        }

        switch(m_orientation) {
        case TileSet::Orthogonal: {
            offset.x = tileOffset.x;
            offset.y += cellSize.y;
            break;
        }
        case TileSet::Isometric: {
            offset.x = (cellSize.x * m_width - (y+2) * cellSize.x) * 0.5f;
            offset.y = (y-1) * cellSize.y * 0.5f;
            offset += tileOffset;
            break;
        }
        case TileSet::Hexagonal: {
            offset.x = (y % 2 == m_staggerIndex) ? 0.0f : cellSize.x * 0.5f + tileOffset.x;
            offset.y += (cellSize.y + m_hexSizeLength) * 0.5f;
            break;
        }
        default: break;
        }
    }

    m_tileMesh->clear();
    m_tileMesh->setUv0(uvs);
    m_tileMesh->setVertices(vertices);
    m_tileMesh->setIndices(indices);
    m_tileMesh->recalcBounds();
}
/*!
    \internal
*/
void TileMap::loadUserData(const VariantMap &data) {
    auto it = data.find(DATA);
    if(it != data.end()) {
        ByteArray array((*it).second.toByteArray());
        m_data.resize(array.size() / sizeof(uint32_t));

        memcpy(m_data.data(), array.data(), array.size());
    }

    it = data.find(TILESET);
    if(it != data.end()) {
        setTileSet(Engine::loadResource<TileSet>((*it).second.toString()));
    }

    m_dirty = true;
}
/*!
    \internal
*/
VariantMap TileMap::saveUserData() const {
    VariantMap result;

    ByteArray array;
    array.resize(m_data.size() * sizeof(uint32_t));
    memcpy(array.data(), m_data.data(), array.size());

    result[DATA] = array;

    string ref = Engine::reference(tileSet());
    if(!ref.empty()) {
        result[TILESET] = ref;
    }

    return result;
}
