#include "navigationsystem.h"

#include <log.h>
#include <engine.h>
#include <timer.h>
#include <world.h>
#include <scene.h>
#include <transform.h>

#include <boxcollider.h>
#include <spherecollider.h>
#include <capsulecollider.h>
#include <meshcollider.h>

#include <systems/resourcesystem.h>

#include <Recast.h>
#include <RecastAlloc.h>
#include <RecastAssert.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourNavMeshBuilder.h>
#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>

#include "components/navigationsurface.h"
#include "components/navigationagent.h"
#include "components/navigationobstacle.h"
#include "components/navigationlink.h"

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

class SimpleTileCacheMeshProcess : public dtTileCacheMeshProcess {
public:
    void process(struct dtNavMeshCreateParams *params, uint8_t *polyAreas, uint16_t *polyFlags) override {
        for(int i = 0; i < params->polyCount; ++i) {
            polyAreas[i] = 1;      // SAMPLE_POLYAREA_GROUND
            polyFlags[i] = 0xFFFF;
        }
    }
};

NavigationSystem::NavigationSystem() : System() {
    PROFILE_FUNCTION();

    NavigationSurface::registerClassFactory(this);
    NavigationAgent::registerClassFactory(this);
    NavigationLink::registerClassFactory(this);
    NavigationObstacle::registerClassFactory(this);

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

    if(m_tileMeshProcessor) {
        delete m_tileMeshProcessor;
        m_tileMeshProcessor = nullptr;
    }

    if(m_tileCache) {
        dtFreeTileCache(m_tileCache);
        m_tileCache = nullptr;
    }

    NavigationSurface::unregisterClassFactory(this);
    NavigationAgent::unregisterClassFactory(this);
    NavigationLink::unregisterClassFactory(this);
    NavigationObstacle::unregisterClassFactory(this);

    NavMesh::unregisterClassFactory(Engine::resourceSystem());
}

bool NavigationSystem::init() {
    PROFILE_FUNCTION();

    m_tileCompressor = new SimpleTileCacheCompressor();
    m_tileMeshProcessor = new SimpleTileCacheMeshProcess();

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

    dtStatus status = m_tileCache->init(&cacheParams, nullptr, m_tileCompressor, m_tileMeshProcessor);
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

    static const uint32_t navSurfTagHash = Mathf::hashString("navsurf");

    for(auto *scene : world->scenes()) {
        if(!scene) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            if(m_sceneNavMeshes.find(scene) != m_sceneNavMeshes.end()) {
                continue;
            }
        }

        Object::ObjectList &surfaces = scene->getObjectsInGroupByHash(navSurfTagHash);

        NavigationSurface *surface = nullptr;
        for(Object *obj : surfaces) {
            surface = dynamic_cast<NavigationSurface *>(obj);
            if(surface) {
                break;
            }
        }

        if(surface) {
            NavMesh *navMesh = surface->navMesh();
            if(navMesh && navMesh->state() == NavMesh::Ready) {
                registerNavMesh(scene, navMesh);
                aInfo() << "Navigation: Registered NavMesh from NavigationSurface for scene: " << scene->name();
                continue;
            }
        }
    }

    if(m_tileCache) {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        float deltaTime = Timer::deltaTime();
        for(auto &pair : m_sceneNavMeshes) {
            NavMesh *navMesh = pair.second;
            if(navMesh && navMesh->navMesh()) {
                m_tileCache->update(deltaTime, navMesh->navMesh());
            }
        }
    }
}

std::vector<Vector3> NavigationSystem::findPath(Scene *scene, const Vector3 &start, const Vector3 &end, uint32_t agentType) {
    std::vector<Vector3> result;

    if(!scene) {
        return result;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);

    NavMesh *navMesh = getNavMesh(scene);
    if(!navMesh || navMesh->state() != NavMesh::Ready) {
        return result;
    }

    dtNavMesh *dtNavMesh = navMesh->navMesh();
    if(!dtNavMesh) {
        return result;
    }

    dtNavMeshQuery query;
    if(dtStatusFailed(query.init(dtNavMesh, 2048))) {
        return result;
    }

    dtQueryFilter filter;
    filter.setIncludeFlags(0xFFFF);
    filter.setExcludeFlags(0);

    dtPolyRef startRef, endRef;
    float startPos[3] = {start.x, start.y, start.z};
    float endPos[3] = {end.x, end.y, end.z};
    float nearestStart[3], nearestEnd[3];
    float halfExtents[3] = {m_agentTypes[agentType].radius, m_agentTypes[agentType].height, m_agentTypes[agentType].radius};

    dtStatus status = query.findNearestPoly(startPos, halfExtents, &filter, &startRef, nearestStart);
    if(dtStatusFailed(status) || !startRef) {
        return result;
    }

    status = query.findNearestPoly(endPos, halfExtents, &filter, &endRef, nearestEnd);
    if(dtStatusFailed(status) || !endRef) {
        return result;
    }

    dtPolyRef polys[256];
    int polyCount;
    status = query.findPath(startRef, endRef, nearestStart, nearestEnd, &filter, polys, &polyCount, 256);
    if(dtStatusFailed(status) || polyCount == 0) {
        return result;
    }

    float straightPath[256 * 3];
    unsigned char straightFlags[256];
    dtPolyRef straightPolys[256];
    int straightCount;

    status = query.findStraightPath(nearestStart, nearestEnd, polys, polyCount,
                                    straightPath, straightFlags, straightPolys, &straightCount, 256);
    if(dtStatusFailed(status) || straightCount == 0) {
        return result;
    }

    for(int i = 0; i < straightCount; ++i) {
        result.push_back(Vector3(straightPath[i * 3], straightPath[i * 3 + 1], straightPath[i * 3 + 2]));
    }

    return result;
}

bool NavigationSystem::registerNavMesh(Scene *scene, NavMesh *navMesh) {
    if(!scene || !navMesh || navMesh->state() != NavMesh::Ready) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);
    unregisterNavMeshLocked(scene);

    m_sceneNavMeshes[scene] = navMesh;

    if(!rebuildTileCacheTiles(navMesh)) {
        m_sceneNavMeshes.erase(scene);
        return false;
    }

    return true;
}

void NavigationSystem::unregisterNavMesh(Scene *scene) {
    if(!scene) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);
    unregisterNavMeshLocked(scene);
}

void NavigationSystem::unregisterNavMeshLocked(Scene *scene) {
    if(!scene) {
        return;
    }

    auto it = m_sceneNavMeshes.find(scene);
    if(it == m_sceneNavMeshes.end()) {
        return;
    }

    NavMesh *navMesh = it->second;
    if(navMesh && navMesh->navMesh() && m_tileCache) {
        dtNavMesh *dtNavMesh = navMesh->navMesh();
        const class dtNavMesh *constNavMesh = static_cast<const class dtNavMesh*>(dtNavMesh);

        for(int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
            const dtMeshTile *tile = constNavMesh->getTile(i);
            if(!tile || !tile->header) {
                continue;
            }

            dtTileRef tileRef = dtNavMesh->getTileRef(tile);
            if(tileRef) {
                m_tileCache->removeTile(tileRef, nullptr, nullptr);
            }
        }
    }

    m_sceneNavMeshes.erase(it);
}

NavMesh *NavigationSystem::getNavMesh(Scene *scene) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    auto it = m_sceneNavMeshes.find(scene);
    if(it == m_sceneNavMeshes.end()) {
        return nullptr;
    }
    return it->second;
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

bool NavigationSystem::rebuildTile(Scene *scene, int tileX, int tileY, const void *tileData, size_t tileDataSize) {
    if(!scene || !m_tileCache) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);

    NavMesh *navMesh = getNavMesh(scene);
    if(!navMesh) {
        return false;
    }

    dtNavMesh *dtNavMesh = navMesh->navMesh();
    if(!dtNavMesh) {
        return false;
    }

    dtTileRef oldTileRef = navMesh->tileRef(tileX, tileY);
    if(oldTileRef) {
        unsigned char *removedData = nullptr;
        int removedDataSize = 0;

        dtStatus status = dtNavMesh->removeTile(oldTileRef, &removedData, &removedDataSize);
        if(dtStatusFailed(status)) {
            return false;
        }

        if(removedData) {
            dtFree(removedData);
        }
    }

    dtTileRef newTileRef;
    dtStatus status = dtNavMesh->addTile((unsigned char*)tileData, tileDataSize, DT_TILE_FREE_DATA, 0, &newTileRef);
    if(dtStatusFailed(status)) {
        return false;
    }

    status = m_tileCache->buildNavMeshTilesAt(tileX, tileY, dtNavMesh);
    if(dtStatusFailed(status)) {
        aWarning() << "Navigation: Failed to build tile (" << tileX << ", " << tileY << ")";
    }

    if(navMesh->query()) {
        navMesh->query()->init(dtNavMesh, 2048);
    }

    return true;
}

void NavigationSystem::invalidateTile(Scene *scene, int tileX, int tileY) {
    if(!scene || !m_tileCache) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);

    NavMesh *navMesh = getNavMesh(scene);
    if(!navMesh) {
        return;
    }

    dtTileRef tileRef = navMesh->tileRef(tileX, tileY);
    if(!tileRef) {
        return;
    }

    invalidateTileByRef(scene, tileRef);
}

void NavigationSystem::invalidateTileByRef(Scene *scene, dtTileRef tileRef) {
    if(!scene || !tileRef || !m_tileCache) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_dataMutex);

    NavMesh *navMesh = getNavMesh(scene);
    if(!navMesh) {
        return;
    }

    dtNavMesh *dtNavMesh = navMesh->navMesh();
    if(!dtNavMesh) {
        return;
    }

    m_tileCache->removeTile(tileRef, nullptr, nullptr);

    unsigned char *removedData = nullptr;
    int removedDataSize = 0;
    dtStatus status = dtNavMesh->removeTile(tileRef, &removedData, &removedDataSize);

    if(dtStatusSucceed(status) && removedData) {
        dtFree(removedData);
    }

    if(navMesh->query()) {
        navMesh->query()->init(dtNavMesh, 2048);
    }
}

bool NavigationSystem::addObstacle(NavigationObstacle &obstacle) {
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

bool NavigationSystem::buildNavMeshFromSurface(NavigationSurface *surface) {
    if(!surface) {
        aError() << "Navigation: Surface is null";
        return false;
    }

    Scene *scene = surface->scene();
    if(!scene) {
        aError() << "Navigation: Surface has no scene";
        return false;
    }

    Vector3Vector vertices;
    std::vector<int> indices;

    if(!collectGeometryFromSurface(surface, vertices, indices)) {
        aError() << "Navigation: Failed to collect geometry from surface";
        return false;
    }

    if(vertices.empty() || indices.empty()) {
        aError() << "Navigation: No geometry collected for surface";
        return false;
    }

    NavMesh *navMesh = Engine::objectCreate<NavMesh>("NavMesh");
    if(!navMesh) {
        aError() << "Navigation: Failed to create NavMesh";
        return false;
    }

    navMesh->setCellSize(surface->cellSize());
    navMesh->setCellHeight(surface->cellHeight());
    navMesh->setTileSize(surface->tileSize());
    navMesh->setOrigin(surface->origin());

    if(!buildNavMeshData(vertices, indices, navMesh)) {
        aError() << "Navigation: Failed to build NavMesh data";
        delete navMesh;
        return false;
    }

    if(navMesh->state() != NavMesh::Ready) {
        aError() << "Navigation: Failed to load NavMesh data";
        delete navMesh;
        return false;
    }

    if(!registerNavMesh(scene, navMesh)) {
        aError() << "Navigation: Failed to register NavMesh";
        delete navMesh;
        return false;
    }

    const_cast<NavigationSurface*>(surface)->setNavMesh(navMesh);

    return true;
}

bool NavigationSystem::buildNavMeshData(const Vector3Vector &vertices, const std::vector<int> &indices, NavMesh *navMesh) {
    if(!navMesh || vertices.empty() || indices.empty()) {
        return false;
    }

    float cellSize = navMesh->cellSize();
    float cellHeight = navMesh->cellHeight();
    int tileSize = navMesh->tileSize();

    const float *floatVertices = vertices[0].v;

    rcConfig config;
    memset(&config, 0, sizeof(config));
    config.cs = cellSize;
    config.ch = cellHeight;
    config.walkableSlopeAngle = m_agentTypes[0].maxSlope;
    config.walkableHeight = m_agentTypes[0].height;
    config.walkableClimb = m_agentTypes[0].maxClimb;
    config.walkableRadius = m_agentTypes[0].radius;
    config.maxEdgeLen = 12.0f;
    config.maxSimplificationError = 1.3f;
    config.minRegionArea = 8.0f;
    config.mergeRegionArea = 20.0f;
    config.maxVertsPerPoly = 6;
    config.detailSampleDist = 6.0f;
    config.detailSampleMaxError = 1.0f;

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
    config.tileSize = tileSize;

    aDebug() << "Navigation: Config - width=" << config.width
             << ", height=" << config.height
             << ", tileSize=" << config.tileSize;

    rcContext ctx;

    rcHeightfield *hf = rcAllocHeightfield();
    if(!hf) {
        aError() << "Navigation: Failed to allocate heightfield";
        return false;
    }

    if(!rcCreateHeightfield(&ctx, *hf, config.width, config.height,
                             config.bmin, config.bmax, config.cs, config.ch)) {
        aError() << "Navigation: Failed to create heightfield";
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
        aError() << "Navigation: Failed to allocate compact heightfield";
        rcFreeHeightField(hf);
        return false;
    }

    if(!rcBuildCompactHeightfield(&ctx, config.walkableHeight, config.walkableClimb, *hf, *chf)) {
        aError() << "Navigation: Failed to build compact heightfield";
        rcFreeHeightField(hf);
        rcFreeCompactHeightfield(chf);
        return false;
    }

    rcFreeHeightField(hf);

    rcErodeWalkableArea(&ctx, config.walkableRadius, *chf);

    rcContourSet *cset = rcAllocContourSet();
    if(!cset) {
        aError() << "Navigation: Failed to allocate contour set";
        rcFreeCompactHeightfield(chf);
        return false;
    }

    rcPolyMesh *pmesh = rcAllocPolyMesh();
    if(!pmesh) {
        aError() << "Navigation: Failed to allocate poly mesh";
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        return false;
    }

    rcPolyMeshDetail *dmesh = rcAllocPolyMeshDetail();
    if(!dmesh) {
        aError() << "Navigation: Failed to allocate poly mesh detail";
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        return false;
    }

    if(!rcBuildDistanceField(&ctx, *chf)) {
        aError() << "Navigation: Failed to build distance field";
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    if(!rcBuildRegions(&ctx, *chf, 0, config.minRegionArea, config.mergeRegionArea)) {
        aError() << "Navigation: Failed to build regions";
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    if(!rcBuildContours(&ctx, *chf, config.maxSimplificationError, config.maxEdgeLen, *cset)) {
        aError() << "Navigation: Failed to build contours";
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    if(!rcBuildPolyMesh(&ctx, *cset, config.maxVertsPerPoly, *pmesh)) {
        aError() << "Navigation: Failed to build poly mesh";
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    rcFreeContourSet(cset);

    if(!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, config.detailSampleDist, config.detailSampleMaxError, *dmesh)) {
        aError() << "Navigation: Failed to build poly mesh detail";
        rcFreeCompactHeightfield(chf);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
        return false;
    }

    rcFreeCompactHeightfield(chf);

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

    unsigned char *navData = nullptr;
    int navDataSize = 0;
    if(!dtCreateNavMeshData(&createParams, &navData, &navDataSize)) {
        aError() << "Navigation: Failed to create NavMesh data";
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
        aError() << "Navigation: Failed to set NavMesh data";
        return false;
    }

    aInfo() << "Navigation: NavMesh data built, size=" << navDataSize;
    return true;
}

bool NavigationSystem::collectGeometryFromSurface(NavigationSurface *surface, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    static uint32_t hash = Mathf::hashString("collider");

    Scene *scene = surface->scene();
    for(auto it : scene->getObjectsInGroupByHash(hash)) {
        Collider *collider = dynamic_cast<Collider *>(it);
        Actor *actor = collider->actor();
        if(!actor || !actor->isEnabled()) continue;
        if(actor->getComponent<NavigationObstacle>() != nullptr) continue;

        BoxCollider *boxCollider = dynamic_cast<BoxCollider *>(collider);
        if(boxCollider && boxCollider->isEnabled()) {
            addBoxColliderGeometry(boxCollider, outVertices, outIndices);
            continue;
        }

        MeshCollider *meshCollider = dynamic_cast<MeshCollider *>(collider);
        if(meshCollider && meshCollider->isEnabled()) {
            addMeshColliderGeometry(meshCollider, outVertices, outIndices);
            continue;
        }

        CapsuleCollider *capsuleCollider = dynamic_cast<CapsuleCollider *>(collider);
        if(capsuleCollider && capsuleCollider->isEnabled()) {
            addCapsuleColliderGeometry(capsuleCollider, outVertices, outIndices);
            continue;
        }

        SphereCollider *sphereCollider = dynamic_cast<SphereCollider *>(collider);
        if(sphereCollider && sphereCollider->isEnabled()) {
            addSphereColliderGeometry(sphereCollider, outVertices, outIndices);
            continue;
        }
    }

    return !outVertices.empty() && !outIndices.empty();
}

void NavigationSystem::addBoxColliderGeometry(BoxCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    Vector3 size = collider->size();
    Vector3 center = collider->center();
    Transform *transform = collider->transform();

    Vector3 localVerts[8] = {
        Vector3(-size.x * 0.5f, -size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f, -size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f, -size.y * 0.5f,  size.z * 0.5f),
        Vector3(-size.x * 0.5f, -size.y * 0.5f,  size.z * 0.5f),
        Vector3(-size.x * 0.5f,  size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f,  size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f,  size.y * 0.5f,  size.z * 0.5f),
        Vector3(-size.x * 0.5f,  size.y * 0.5f,  size.z * 0.5f)
    };

    size_t startIndex = outVertices.size();

    for(int i = 0; i < 8; ++i) {
        Vector3 worldPos = center + localVerts[i];
        outVertices.push_back(transform->worldTransform() * worldPos);
    }

    static const uint32_t indices[] = {
        0, 1, 2,  0, 2, 3,
        4, 6, 5,  4, 7, 6,
        0, 4, 5,  0, 5, 1,
        1, 5, 6,  1, 6, 2,
        2, 6, 7,  2, 7, 3,
        3, 7, 4,  3, 4, 0
    };

    for(uint32_t idx : indices) {
        outIndices.push_back(static_cast<uint32_t>(startIndex + idx));
    }
}

void NavigationSystem::addMeshColliderGeometry(MeshCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    Mesh *mesh = collider->mesh();
    if(!mesh) {
        return;
    }

    Transform *transform = collider->transform();

    size_t startIndex = outVertices.size();

    const auto &localVerts = mesh->vertices();
    for(const Vector3 &v : localVerts) {
        outVertices.push_back(transform->worldTransform() * v);
    }

    const auto &localIndices = mesh->indices();
    for(uint32_t idx : localIndices) {
        outIndices.push_back(static_cast<uint32_t>(startIndex + idx));
    }
}

void NavigationSystem::addCapsuleColliderGeometry(CapsuleCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    /// \todo: implement this
}

void NavigationSystem::addSphereColliderGeometry(SphereCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    /// \todo: implement this
}
