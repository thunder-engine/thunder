#include "resources/navmesh.h"

#include <log.h>
#include <engine.h>
#include <systems/resourcesystem.h>

#include <Recast.h>
#include <RecastAlloc.h>
#include <RecastAssert.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

NavMesh::NavMesh() :
        Resource() {
}

NavMesh::~NavMesh() {
    cleanup();
}

void NavMesh::setCellSize(float size) {
    m_cellSize = size;
}

void NavMesh::setCellHeight(float height) {
    m_cellHeight = height;
}

void NavMesh::setTileSize(int size) {
    m_tileSize = size;
}

void NavMesh::setOrigin(const Vector3 &origin) {
    m_origin = origin;
}

void NavMesh::loadUserData(const VariantMap &data) {
    if(m_navMesh) {
        cleanup();
    }

    auto it = data.find("config");
    if(it != data.end()) {
        VariantList config = it->second.toList();
        auto field = config.begin();

        m_cellSize = field->toFloat();
        ++field;
        m_cellHeight = field->toFloat();
        ++field;
        m_tileSize = field->toInt();
        ++field;
        m_origin = field->toVector3();
    }

    it = data.find("tiles");
    if(it != data.end()) {
        for(auto &tile : it->second.toList()) {
            ByteArray tileData = tile.toByteArray();
            if(!setData(tileData)) {
                return;
            }
        }
    }

    m_query = dtAllocNavMeshQuery();
    if(m_query) {
        m_query->init(m_navMesh, 2048);
    }

    switchState(Ready);
}

VariantMap NavMesh::saveUserData() const {
    VariantMap result;

    if(m_navMesh) {
        VariantList config;
        config.push_back(m_cellSize);
        config.push_back(m_cellHeight);
        config.push_back(m_tileSize);
        config.push_back(m_origin);

        result["config"] = config;

        VariantList tiles;
        const dtNavMesh *constNavMesh = static_cast<const dtNavMesh*>(m_navMesh);
        for(int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
            const dtMeshTile *tile = constNavMesh->getTile(i);
            if(!tile || !tile->header || !tile->data) {
                continue;
            }

            result["tiles"] = ByteArray(tile->data, tile->data + tile->dataSize);
            break;
        }
    }

    return result;
}

bool NavMesh::initNavMesh() {
    dtNavMeshParams params;
    memset(&params, 0, sizeof(params));
    params.orig[0] = m_origin.x;
    params.orig[1] = m_origin.y;
    params.orig[2] = m_origin.z;
    params.tileWidth = m_tileSize * m_cellSize;
    params.tileHeight = m_tileSize * m_cellHeight;
    params.maxTiles = 1;
    params.maxPolys = 32768;

    m_navMesh = dtAllocNavMesh();
    if(!m_navMesh) {
        return false;
    }

    dtStatus status = m_navMesh->init(&params);
    if(dtStatusFailed(status)) {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
        return false;
    }

    return true;
}

void NavMesh::cleanup() {
    if(m_query) {
        dtFreeNavMeshQuery(m_query);
        m_query = nullptr;
    }

    if(m_navMesh) {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = nullptr;
    }
}

dtTileRef NavMesh::tileRef(int tileX, int tileY) const {
    if(!m_navMesh) {
        return 0;
    }

    const dtMeshTile *tile = m_navMesh->getTileAt(tileX, tileY, 0);
    if(!tile) {
        return 0;
    }

    return m_navMesh->getTileRef(tile);
}

bool NavMesh::setData(ByteArray &data) {
    if(!m_navMesh && !initNavMesh()) {
        return false;
    }

    dtTileRef tileRef;
    dtStatus status = m_navMesh->addTile(data.data(), data.size(), DT_TILE_FREE_DATA, 0, &tileRef);
    if(dtStatusFailed(status)) {
        return false;
    }
    return true;
}
