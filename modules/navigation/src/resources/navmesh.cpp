#include "resources/navmesh.h"

#include <log.h>
#include <engine.h>
#include <systems/resourcesystem.h>

#include <cstring>

#include <Recast.h>
#include <RecastAlloc.h>
#include <RecastAssert.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

/*!
    \class NavMesh
    \brief The NavMesh class stores a Detour navigation mesh resource.
    \inmodule Navigation

    A navigation mesh resource owns the Detour mesh and its query object. It
    can load and save serialized tile data and provides access to the mesh,
    query object, and tile reference used by the navigation system.

    \sa NavMeshSurface, NavMeshAgent, NavigationSystem
*/

/*!
    Constructs an empty navigation mesh resource.
*/
NavMesh::NavMesh() :
        Resource(),
        m_navMesh(nullptr),
        m_query(nullptr),
        m_tileRef(0) {
}

/*!
    Destroys the navigation mesh resource and releases its Detour objects.
*/
NavMesh::~NavMesh() {
    cleanup();
}

/*!
    Returns the underlying Detour navigation mesh, or nullptr when it is not
    loaded.
*/
dtNavMesh *NavMesh::navMesh() const {
    return m_navMesh;
}

/*!
    Returns the Detour navigation query object, or nullptr when it is not
    initialized.
*/
dtNavMeshQuery *NavMesh::query() const {
    return m_query;
}

/*!
    Loads serialized navigation mesh tile \a data from resource user data.

    The data map contains the serialized resource user data and its tile data.
*/
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

/*!
    Saves the first available navigation mesh tile as resource user data.
*/
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

/*!
    Releases the Detour navigation mesh and query objects owned by the resource.
*/
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

/*!
    Returns the Detour reference for the tile at coordinates \a tileX and \a tileY.

    Returns zero when the navigation mesh is not loaded or no tile exists at
    the requested coordinates.
*/
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

/*!
    Loads serialized Detour tile data into the navigation mesh.

    The \a data replaces the currently loaded tile. The method returns true when
    it is added successfully, or false when the data is invalid or Detour
    rejects the tile.
*/
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
