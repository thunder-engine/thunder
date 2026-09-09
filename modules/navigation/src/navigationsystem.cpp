#include "navigationsystem.h"

#include <log.h>
#include <engine.h>
#include <timer.h>
#include <world.h>
#include <transform.h>

#include <systems/resourcesystem.h>

#include <DetourNavMeshQuery.h>
#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>

#include "components/navmeshsurface.h"
#include "components/navmeshagent.h"
#include "components/navmeshobstacle.h"
#include "components/navmeshlink.h"

#include "resources/navmesh.h"

std::vector<AgentType> NavigationSystem::s_agentTypes = {AgentType()};

class SimpleTileCacheCompressor : public dtTileCacheCompressor {
public:
    int maxCompressedSize(const int bufferSize) override {
        return bufferSize + 1024;
    }

    dtStatus compress(const uint8_t *buffer, const int bufferSize, uint8_t *compressed, const int maxCompressedSize, int *compressedSize) override {
        if(bufferSize > maxCompressedSize) {
            return DT_FAILURE;
        }
        memcpy(compressed, buffer, bufferSize);
        *compressedSize = bufferSize;
        return DT_SUCCESS;
    }

    dtStatus decompress(const uint8_t *compressed, const int compressedSize, uint8_t *buffer, const int maxBufferSize, int *bufferSize) override {
        if(compressedSize > maxBufferSize) {
            return DT_FAILURE;
        }
        memcpy(buffer, compressed, compressedSize);
        *bufferSize = compressedSize;
        return DT_SUCCESS;
    }
};

NavigationSystem::NavigationSystem() :
        System() {
    PROFILE_FUNCTION();

    NavMeshSurface::registerClassFactory(this);
    NavMeshAgent::registerClassFactory(this);
    NavMeshLink::registerClassFactory(this);
    NavMeshObstacle::registerClassFactory(this);

    NavMesh::registerClassFactory(Engine::resourceSystem());

    setName("Navigation");

}

NavigationSystem::~NavigationSystem() {
    PROFILE_FUNCTION();

    if(m_tileCompressor) {
        delete m_tileCompressor;
        m_tileCompressor = nullptr;
    }

    if(m_tileCache) {
        dtFreeTileCache(m_tileCache);
        m_tileCache = nullptr;
    }

    NavMeshSurface::unregisterClassFactory(this);
    NavMeshAgent::unregisterClassFactory(this);
    NavMeshLink::unregisterClassFactory(this);
    NavMeshObstacle::unregisterClassFactory(this);

    NavMesh::unregisterClassFactory(Engine::resourceSystem());
}

bool NavigationSystem::init() {
    PROFILE_FUNCTION();

    VariantMap navigation = Engine::value("navigation").toMap();
    VariantList types = navigation["navmesh"].toList();
    std::vector<AgentType> configuredTypes;
    for(const Variant &item : types) {
        VariantMap data = item.toMap();
        if(data.empty()) {
            continue;
        }

        AgentType type;
        type.name = data["name"].toString();
        type.height = data["height"].toFloat();
        type.radius = data["radius"].toFloat();
        type.maxClimb = data["maxClimb"].toFloat();
        type.maxSlope = data["maxSlope"].toFloat();

        configuredTypes.push_back(type);
    }
    if(!configuredTypes.empty()) {
        s_agentTypes = configuredTypes;
    }

    m_tileCompressor = new SimpleTileCacheCompressor();

    dtTileCacheParams cacheParams;
    memset(&cacheParams, 0, sizeof(cacheParams));
    cacheParams.orig[0] = 0.0f;
    cacheParams.orig[1] = 0.0f;
    cacheParams.orig[2] = 0.0f;
    cacheParams.cs = 0.3f;
    cacheParams.ch = 0.2f;
    cacheParams.walkableHeight = s_agentTypes[0].height;
    cacheParams.walkableRadius = s_agentTypes[0].radius;
    cacheParams.walkableClimb = s_agentTypes[0].maxClimb;
    cacheParams.width = 1024;
    cacheParams.height = 1024;
    cacheParams.maxTiles = 1024 * 1024;
    cacheParams.maxObstacles = 32768;

    m_tileCache = dtAllocTileCache();
    if(!m_tileCache) {
        aError() << "Navigation: Failed to allocate global dtTileCache";
        return false;
    }

    dtStatus status = m_tileCache->init(&cacheParams, nullptr, m_tileCompressor, nullptr);
    if(dtStatusFailed(status)) {
        aError() << "Navigation: Failed to init global dtTileCache: 0x" << status;
        dtFreeTileCache(m_tileCache);
        m_tileCache = nullptr;
        return false;
    }

    aInfo() << "Navigation: Global TileCache initialized";
    return true;
}

void NavigationSystem::update(World *world) {
    if(!world) {
        return;
    }

    if(m_tileCache) {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        float deltaTime = Timer::deltaTime();
        for(auto &pair : m_surfaceNavMeshes) {
            NavMesh *navMesh = pair.second;
            if(navMesh && navMesh->navMesh()) {
                m_tileCache->update(deltaTime, navMesh->navMesh());
            }
        }
    }
}

std::vector<Vector3> NavigationSystem::findPath(NavMeshAgent &agent, const Vector3 &target) {
    std::vector<Vector3> result;

    Vector3 start = agent.transform()->worldPosition();
    int agentType = agent.agentType();

    NavMesh *startNavMesh = findNavMeshAtPosition(start, agentType);
    if(!startNavMesh || startNavMesh->state() != NavMesh::Ready) {
        aWarning() << "findPath: No NavMesh at start position";
        return result;
    }

    NavMesh *endNavMesh = findNavMeshAtPosition(target, agentType);
    if(!endNavMesh || endNavMesh->state() != NavMesh::Ready) {
        aWarning() << "findPath: No NavMesh at end position";
        return result;
    }

    if(startNavMesh == endNavMesh) {
        return findPathOnNavMesh(startNavMesh, start, target, agentType);
    }

    aInfo() << "findPath: Cross-NavMesh path from " << startNavMesh->name()
            << " to " << endNavMesh->name();

    // TODO: Implement search via off-network communication between NavMeshes
    // For now, we're simply searching for a path to the closest point on startNavMesh
    // and from the closest point on endNavMesh

    return result;
}

std::vector<Vector3> NavigationSystem::findPathOnNavMesh(NavMesh *navMesh, const Vector3 &start, const Vector3 &end, int agentType) {
    std::vector<Vector3> result;

    dtNavMesh *dtNavMesh = navMesh->navMesh();
    if(!dtNavMesh) return result;

    dtNavMeshQuery query;
    if(dtStatusFailed(query.init(dtNavMesh, 2048))) return result;

    dtQueryFilter filter;
    filter.setIncludeFlags(0xFFFF);
    filter.setExcludeFlags(0);

    dtPolyRef startRef, endRef;
    float startPos[3] = {start.x, start.y, start.z};
    float endPos[3] = {end.x, end.y, end.z};
    float nearestStart[3], nearestEnd[3];
    float halfExtents[3] = {
        s_agentTypes[agentType].radius,
        s_agentTypes[agentType].height,
        s_agentTypes[agentType].radius
    };

    if(dtStatusFailed(query.findNearestPoly(startPos, halfExtents, &filter, &startRef, nearestStart))) {
        return result;
    }

    if(dtStatusFailed(query.findNearestPoly(endPos, halfExtents, &filter, &endRef, nearestEnd))) {
        return result;
    }

    if(!startRef || !endRef) return result;

    dtPolyRef polys[256];
    int polyCount;
    if(dtStatusFailed(query.findPath(startRef, endRef, nearestStart, nearestEnd, &filter, polys, &polyCount, 256))) {
        return result;
    }

    if(polyCount == 0) return result;

    float straightPath[256 * 3];
    unsigned char straightFlags[256];
    dtPolyRef straightPolys[256];
    int straightCount;

    if(dtStatusFailed(query.findStraightPath(nearestStart, nearestEnd, polys, polyCount,
                                              straightPath, straightFlags, straightPolys, &straightCount, 256))) {
        return result;
    }

    for(int i = 0; i < straightCount; ++i) {
        result.push_back(Vector3(straightPath[i * 3], straightPath[i * 3 + 1], straightPath[i * 3 + 2]));
    }

    return result;
}

bool NavigationSystem::registerNavMesh(NavMeshSurface &surface) {
    unregisterNavMesh(surface);

    NavMesh *navMesh = surface.navMesh();
    if(!navMesh || navMesh->state() != NavMesh::Ready) {
        return false;
    }

    if(rebuildTileCacheTiles(navMesh)) {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        m_surfaceNavMeshes[&surface] = navMesh;
        return true;
    }

    return false;
}

void NavigationSystem::unregisterNavMesh(NavMeshSurface &surface) {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    auto it = m_surfaceNavMeshes.find(&surface);
    if(it == m_surfaceNavMeshes.end()) {
        return;
    }

    NavMesh *navMesh = it->second;
    if(navMesh && navMesh->navMesh() && m_tileCache) {
        dtNavMesh *dtNavMesh = navMesh->navMesh();
        const class dtNavMesh *constNavMesh = static_cast<const class dtNavMesh*>(dtNavMesh);

        for(int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
            const dtMeshTile *tile = constNavMesh->getTile(i);
            if(!tile || !tile->header) continue;

            dtTileRef tileRef = dtNavMesh->getTileRef(tile);
            if(tileRef) {
                m_tileCache->removeTile(tileRef, nullptr, nullptr);
            }
        }
    }

    m_surfaceNavMeshes.erase(it);
}

bool NavigationSystem::rebuildTileCacheTiles(NavMesh *navMesh) {
    if(!m_tileCache || !navMesh) {
        return false;
    }

    dtNavMesh *dtNavMesh = navMesh->navMesh();
    if(!dtNavMesh) {
        return false;
    }

    const class dtNavMesh *constNavMesh = static_cast<const class dtNavMesh*>(dtNavMesh);
    for(int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
        const dtMeshTile *tile = constNavMesh->getTile(i);
        if(!tile || !tile->header) {
            continue;
        }

        dtStatus status = m_tileCache->buildNavMeshTilesAt(tile->header->x, tile->header->y, dtNavMesh);
        if(dtStatusFailed(status)) {
            aWarning() << "Navigation: Failed to build tile (" << tile->header->x << ", " << tile->header->y << ")";
            return false;
        }
    }

    return true;
}

NavMesh *NavigationSystem::findNavMeshAtPosition(const Vector3 &position, int agentType) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    for(auto &pair : m_surfaceNavMeshes) {
        NavMeshSurface *surface = pair.first;
        if(!surface || !surface->isEnabled()) continue;

        if(surface->agentType() != agentType) continue;

        NavMesh *navMesh = pair.second;
        if(!navMesh || navMesh->state() != NavMesh::Ready) continue;

        dtNavMesh *dtNavMesh = navMesh->navMesh();
        if(!dtNavMesh) continue;

        dtNavMeshQuery query;
        if(dtStatusFailed(query.init(dtNavMesh, 2048))) continue;

        dtQueryFilter filter;
        filter.setIncludeFlags(0xFFFF);
        filter.setExcludeFlags(0);

        float pos[3] = {position.x, position.y, position.z};
        float halfExtents[3] = {1.0f, 10.0f, 1.0f};

        dtPolyRef ref;
        float nearest[3];
        if(dtStatusSucceed(query.findNearestPoly(pos, halfExtents, &filter, &ref, nearest))) {
            if(ref != 0) {
                return navMesh;
            }
        }
    }

    return nullptr;
}

uint32_t NavigationSystem::addObstacle(NavMeshObstacle &obstacle) {
    if(!m_tileCache) {
        return 0;
    }

    Transform *transform = obstacle.transform();
    Vector3 position = transform->position();

    std::lock_guard<std::mutex> lock(m_dataMutex);

    dtObstacleRef obstacleRef;
    dtStatus status = m_tileCache->addObstacle(position.v, obstacle.radius(), obstacle.height(), &obstacleRef);
    if(dtStatusFailed(status)) {
        return 0;
    }

    return static_cast<uint32_t>(obstacleRef);
}

bool NavigationSystem::removeObstacle(uint32_t obstacleId) {
    if(!m_tileCache) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);

    dtObstacleRef obstacleRef = static_cast<dtObstacleRef>(obstacleId);
    dtStatus status = m_tileCache->removeObstacle(obstacleRef);

    if(dtStatusFailed(status)) {
        return false;
    }

    return true;
}

void NavigationSystem::setAgentType(int index, const AgentType &type) {
    if(index >= 0 && index < static_cast<int>(s_agentTypes.size())) {
        s_agentTypes[index] = type;
    } else if(index == static_cast<int>(s_agentTypes.size())) {
        s_agentTypes.push_back(type);
    } else {
        return;
    }
}

AgentType NavigationSystem::agentType(int index) {
    if(index >= 0 && index < static_cast<int>(s_agentTypes.size())) {
        return s_agentTypes[index];
    }
    return AgentType();
}
