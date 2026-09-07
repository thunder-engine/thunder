#include "navigationsystem.h"

#include <log.h>
#include <engine.h>
#include <timer.h>
#include <world.h>
#include <scene.h>
#include <transform.h>

#include <systems/resourcesystem.h>

#include <Recast.h>
#include <RecastAlloc.h>
#include <RecastAssert.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourNavMeshBuilder.h>
#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>

#include "components/navmeshsurface.h"
#include "components/navmeshagent.h"
#include "components/navmeshobstacle.h"
#include "components/navmeshlink.h"

#include "resources/navmesh.h"

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

    m_agentTypes.push_back(AgentType());
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

    m_tileCompressor = new SimpleTileCacheCompressor();

    dtTileCacheParams cacheParams;
    memset(&cacheParams, 0, sizeof(cacheParams));
    cacheParams.orig[0] = 0.0f;
    cacheParams.orig[1] = 0.0f;
    cacheParams.orig[2] = 0.0f;
    cacheParams.cs = 0.3f;
    cacheParams.ch = 0.2f;
    cacheParams.walkableHeight = m_agentTypes[0].height;
    cacheParams.walkableRadius = m_agentTypes[0].radius;
    cacheParams.walkableClimb = m_agentTypes[0].maxClimb;
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

std::vector<Vector3> NavigationSystem::findPath(NavMeshAgent &agent) {
    std::vector<Vector3> result;

    Vector3 start = agent.transform()->worldPosition();
    Vector3 end = agent.target();
    int agentType = agent.agentType();

    NavMesh *startNavMesh = findNavMeshAtPosition(start, agentType);
    if(!startNavMesh || startNavMesh->state() != NavMesh::Ready) {
        aWarning() << "findPath: No NavMesh at start position";
        return result;
    }

    NavMesh *endNavMesh = findNavMeshAtPosition(end, agentType);
    if(!endNavMesh || endNavMesh->state() != NavMesh::Ready) {
        aWarning() << "findPath: No NavMesh at end position";
        return result;
    }

    if(startNavMesh == endNavMesh) {
        return findPathOnNavMesh(startNavMesh, start, end, agentType);
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
        m_agentTypes[agentType].radius,
        m_agentTypes[agentType].height,
        m_agentTypes[agentType].radius
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

bool NavigationSystem::addObstacle(NavMeshObstacle &obstacle) {
    if(!m_tileCache) {
        return false;
    }

    Transform *transform = obstacle.transform();
    Vector3 position = transform->position();

    std::lock_guard<std::mutex> lock(m_dataMutex);

    dtObstacleRef obstacleRef;
    dtStatus status = m_tileCache->addObstacle(position.v, obstacle.radius(), obstacle.height(), &obstacleRef);
    if(dtStatusFailed(status)) {
        return false;
    }

    obstacle.setObstacleRef(static_cast<uint32_t>(obstacleRef));
    return true;
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

bool NavigationSystem::buildNavMeshFromSurface(NavMeshSurface &surface) {
    Vector3Vector vertices;
    std::vector<int> indices;

    if(!surface.collectGeometry(vertices, indices)) {
        aError() << "Navigation: Failed to collect geometry from surface";
        return false;
    }

    if(!buildNavMeshData(surface, vertices, indices)) {
        aError() << "Navigation: Failed to build NavMesh data";
        return false;
    }

    return true;
}

AgentType NavigationSystem::agentType(int index) const {
    if(index < m_agentTypes.size()) {
        return m_agentTypes[index];
    }
    return AgentType();
}

bool NavigationSystem::buildNavMeshData(NavMeshSurface &surface, const Vector3Vector &vertices, const std::vector<int> &indices) {
    if(vertices.empty() || indices.empty()) {
        return false;
    }

    int agentType = surface.agentType();

    NavMesh *navMesh = surface.navMesh();
    if(navMesh == nullptr) {
        navMesh = Engine::objectCreate<NavMesh>(surface.actor()->name());
        if(!navMesh) {
            aError() << "Navigation: Failed to create NavMesh";
            return false;
        }
    }

    rcConfig config;
    memset(&config, 0, sizeof(config));
    config.cs = m_agentTypes[agentType].radius / 3.0f;
    config.ch = config.cs / 2.0f;
    config.walkableSlopeAngle = m_agentTypes[agentType].maxSlope;
    config.walkableHeight = m_agentTypes[agentType].height / config.ch;
    config.walkableClimb = m_agentTypes[agentType].maxClimb / config.ch;
    config.walkableRadius = m_agentTypes[agentType].radius / config.cs;
    config.maxEdgeLen = 12.0f;
    config.maxSimplificationError = 1.3f;
    config.minRegionArea = 8.0f;
    config.mergeRegionArea = 20.0f;
    config.maxVertsPerPoly = 6;
    config.detailSampleDist = 6.0f;
    config.detailSampleMaxError = 1.0f;

    const float *floatVertices = vertices[0].v;
    float bmin[3], bmax[3];
    rcCalcBounds(floatVertices, (int)vertices.size(), bmin, bmax);

    const float expand = 2.0f;
    bmin[0] -= expand;
    bmin[1] -= expand;
    bmin[2] -= expand;
    bmax[0] += expand;
    bmax[1] += expand;
    bmax[2] += expand;

    config.bmin[0] = bmin[0];
    config.bmin[1] = bmin[1];
    config.bmin[2] = bmin[2];
    config.bmax[0] = bmax[0];
    config.bmax[1] = bmax[1];
    config.bmax[2] = bmax[2];

    config.width = (int)((config.bmax[0] - config.bmin[0]) / config.cs + 0.5f);
    config.height = (int)((config.bmax[2] - config.bmin[2]) / config.cs + 0.5f);
    config.tileSize = surface.tileSize();

    rcContext ctx;

    rcHeightfield *hf = rcAllocHeightfield();
    if(!hf) {
        return false;
    }

    if(!rcCreateHeightfield(&ctx, *hf, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch)) {
        rcFreeHeightField(hf);
        return false;
    }

    int numTris = (int)indices.size() / 3;
    std::vector<unsigned char> triAreaIDs(numTris, 1);

    rcRasterizeTriangles(&ctx, floatVertices, (int)vertices.size(),
                         indices.data(), triAreaIDs.data(), (int)indices.size() / 3,
                         *hf, config.walkableClimb);

    rcFilterLowHangingWalkableObstacles(&ctx, config.walkableClimb, *hf);
    rcFilterLedgeSpans(&ctx, config.walkableHeight, config.walkableClimb, *hf);
    rcFilterWalkableLowHeightSpans(&ctx, config.walkableHeight, *hf);

    rcCompactHeightfield *chf = rcAllocCompactHeightfield();
    if(!chf) {
        rcFreeHeightField(hf);
        return false;
    }

    if(!rcBuildCompactHeightfield(&ctx, config.walkableHeight, config.walkableClimb, *hf, *chf)) {
        rcFreeHeightField(hf);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    rcFreeHeightField(hf);

    rcErodeWalkableArea(&ctx, config.walkableRadius, *chf);

    rcContourSet *cset = rcAllocContourSet();
    if(!cset) {
        rcFreeCompactHeightfield(chf);
        return false;
    }

    rcPolyMesh *pmesh = rcAllocPolyMesh();
    if(!pmesh) {
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        return false;
    }

    rcPolyMeshDetail *dmesh = rcAllocPolyMeshDetail();
    if(!dmesh) {
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        return false;
    }

    if(!rcBuildDistanceField(&ctx, *chf)) {
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    if(!rcBuildRegions(&ctx, *chf, 0, config.minRegionArea, config.mergeRegionArea)) {
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    if(!rcBuildContours(&ctx, *chf, config.maxSimplificationError, config.maxEdgeLen, *cset)) {
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    if(!rcBuildPolyMesh(&ctx, *cset, config.maxVertsPerPoly, *pmesh)) {
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    rcFreeContourSet(cset);

    if(!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, config.detailSampleDist, config.detailSampleMaxError, *dmesh)) {
        rcFreeCompactHeightfield(chf);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    rcFreeCompactHeightfield(chf);

    for(int i = 0; i < pmesh->npolys; ++i) {
        pmesh->flags[i] = 0xFFFF;
        pmesh->areas[i] = 1;
    }

    Scene *scene = surface.scene();
    std::vector<float> offMeshVerts;
    std::vector<float> offMeshRadii;
    std::vector<uint32_t> offMeshUserIds;
    std::vector<uint16_t> offMeshFlags;
    std::vector<uint8_t> offMeshAreas;
    std::vector<uint8_t> offMeshDir;

    if(scene) {
        static const uint32_t linkHash = Mathf::hashString("navmeshlink");
        for(Object *obj : scene->getObjectsInGroupByHash(linkHash)) {
            NavMeshLink *link = dynamic_cast<NavMeshLink *>(obj);
            if(!link || !link->isEnabled()) continue;

            Vector3 worldPosition = link->transform()->worldPosition();

            Vector3 start = worldPosition + link->startPoint();
            Vector3 end = worldPosition + link->endPoint();
            bool bidirectional = link->isBidirectional();

            offMeshVerts.push_back(start.x);
            offMeshVerts.push_back(start.y);
            offMeshVerts.push_back(start.z);
            offMeshVerts.push_back(end.x);
            offMeshVerts.push_back(end.y);
            offMeshVerts.push_back(end.z);

            offMeshFlags.push_back(0xFFFF);
            offMeshFlags.push_back(0xFFFF);

            offMeshAreas.push_back(1);
            offMeshAreas.push_back(1);

            offMeshDir.push_back(bidirectional ? 1 : 0);
            offMeshDir.push_back(bidirectional ? 1 : 0);

            offMeshRadii.push_back(m_agentTypes[agentType].radius);
            offMeshRadii.push_back(m_agentTypes[agentType].radius);

            offMeshUserIds.push_back(0);
            offMeshUserIds.push_back(0);
        }

        if(!offMeshVerts.empty()) {
            aInfo() << "Navigation: Adding " << offMeshVerts.size() / 6 << " off-mesh links";
        }
    }

    dtNavMeshCreateParams createParams;
    memset(&createParams, 0, sizeof(createParams));
    createParams.verts = pmesh->verts;
    createParams.vertCount = pmesh->nverts;
    createParams.polys = pmesh->polys;
    createParams.polyAreas = pmesh->areas;
    createParams.polyFlags = pmesh->flags;
    createParams.polyCount = pmesh->npolys;
    createParams.nvp = pmesh->nvp;
    createParams.detailMeshes = dmesh->meshes;
    createParams.detailVerts = dmesh->verts;
    createParams.detailVertsCount = dmesh->nverts;
    createParams.detailTris = dmesh->tris;
    createParams.detailTriCount = dmesh->ntris;
    createParams.walkableHeight = config.walkableHeight;
    createParams.walkableRadius = config.walkableRadius;
    createParams.walkableClimb = config.walkableClimb;
    createParams.bmin[0] = pmesh->bmin[0];
    createParams.bmin[1] = pmesh->bmin[1];
    createParams.bmin[2] = pmesh->bmin[2];
    createParams.bmax[0] = pmesh->bmax[0];
    createParams.bmax[1] = pmesh->bmax[1];
    createParams.bmax[2] = pmesh->bmax[2];
    createParams.cs = config.cs;
    createParams.ch = config.ch;
    createParams.buildBvTree = true;

    if(!offMeshVerts.empty()) {
        createParams.offMeshConVerts = offMeshVerts.data();
        createParams.offMeshConRad = offMeshRadii.data();
        createParams.offMeshConFlags = offMeshFlags.data();
        createParams.offMeshConAreas = offMeshAreas.data();
        createParams.offMeshConDir = offMeshDir.data();
        createParams.offMeshConUserID = offMeshUserIds.data();
        createParams.offMeshConCount = offMeshVerts.size() / 6;
    }

    unsigned char *navData = nullptr;
    int navDataSize = 0;
    if(!dtCreateNavMeshData(&createParams, &navData, &navDataSize)) {
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    rcFreePolyMesh(pmesh);
    rcFreePolyMeshDetail(dmesh);

    ByteArray outData;
    outData.assign(navData, navData + navDataSize);
    dtFree(navData);

    if(!navMesh->setData(outData)) {
        return false;
    }

    surface.setNavMesh(navMesh);

    return true;
}
