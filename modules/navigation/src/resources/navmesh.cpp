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
        Resource(),
        m_navMesh(nullptr),
        m_query(nullptr),
        m_tileRef(0) {
}

NavMesh::~NavMesh() {
    cleanup();
}

void NavMesh::loadUserData(const VariantMap &data) {
    if(m_navMesh) {
        cleanup();
    }

    auto it = data.find("tile");
    if(it != data.end() && !setData(it->second.toByteArray())) {
        return;
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
        const dtNavMesh *constNavMesh = static_cast<const dtNavMesh*>(m_navMesh);
        for(int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
            const dtMeshTile *tile = constNavMesh->getTile(i);
            if(!tile || !tile->header || !tile->data) {
                continue;
            }

            result["tile"] = ByteArray(tile->data, tile->data + tile->dataSize);
            break;
        }
    }

    return result;
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

    m_tileRef = 0;
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

bool NavMesh::setData(const ByteArray &data) {
    if(!m_navMesh) {
        m_navMesh = dtAllocNavMesh();
        if(!m_navMesh) {
            return false;
        }

        if(data.size() < sizeof(dtMeshHeader)) {
            return false;
        }

        const dtMeshHeader *header = reinterpret_cast<const dtMeshHeader*>(data.data());

        dtNavMeshParams params;
        memset(&params, 0, sizeof(params));
        params.orig[0] = header->bmin[0];
        params.orig[1] = header->bmin[1];
        params.orig[2] = header->bmin[2];
        params.tileWidth = header->bmax[0] - header->bmin[0];
        params.tileHeight = header->bmax[2] - header->bmin[2];
        params.maxTiles = 1;
        params.maxPolys = 32768;

        dtStatus status = m_navMesh->init(&params);
        if(dtStatusFailed(status)) {
            dtFreeNavMesh(m_navMesh);
            m_navMesh = nullptr;
            return false;
        }
    }

    if(m_tileRef) {
        m_navMesh->removeTile(m_tileRef, nullptr, nullptr);
        m_tileRef = 0;
    }

    unsigned char *tileDataCopy = reinterpret_cast<unsigned char *>(dtAlloc(data.size(), DT_ALLOC_PERM));
    memcpy(tileDataCopy, data.data(), data.size());

    dtStatus status = m_navMesh->addTile(tileDataCopy, data.size(), DT_TILE_FREE_DATA, 0, &m_tileRef);
    if(dtStatusFailed(status)) {
        return false;
    }
    return true;
}
