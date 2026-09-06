#ifndef NAVIGATIONSYSTEM_H
#define NAVIGATIONSYSTEM_H

#include <system.h>

#include <DetourNavMesh.h>

#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>

class Scene;
class NavMesh;
class NavigationObstacle;
class NavigationLink;

class BoxCollider;
class MeshCollider;
class CapsuleCollider;
class SphereCollider;
class NavigationSurface;

struct rcCompactHeightfield;
struct rcConfig;

struct AgentType {
    float height = 2.0f;
    float radius = 0.5f;
    float maxClimb = 0.75f;
    float maxSlope = 45.0f;
};

class NavigationSystem : public System {
public:
    NavigationSystem();
    ~NavigationSystem();

    bool init() override;

    void update(World *world) override;

    int threadPolicy() const override { return Pool; };

    std::vector<Vector3> findPath(Scene *scene, const Vector3 &start, const Vector3 &end, uint32_t agentType);

    bool registerNavMesh(Scene *scene, NavMesh *navMesh);
    void unregisterNavMesh(Scene *scene);
    NavMesh *getNavMesh(Scene *scene) const;

    bool rebuildTile(Scene *scene, int tileX, int tileY, const void *tileData, size_t tileDataSize);
    void invalidateTile(Scene *scene, int tileX, int tileY);
    void invalidateTileByRef(Scene *scene, dtTileRef tileRef);

    bool addObstacle(NavigationObstacle &obstacle);
    bool removeObstacle(uint32_t obstacleId);

    bool buildNavMeshFromSurface(NavigationSurface *surface);

    AgentType agentType(int index) const;

protected:
    void unregisterNavMeshLocked(Scene *scene);

    bool collectGeometryFromSurface(NavigationSurface *surface, Vector3Vector &outVertices, std::vector<int> &outIndices);

    bool buildNavMeshData(NavigationSurface *surface, const Vector3Vector &vertices, const std::vector<int> &indices, NavMesh *navMesh);

    void addBoxColliderGeometry(BoxCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);
    void addMeshColliderGeometry(MeshCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);
    void addCapsuleColliderGeometry(CapsuleCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);
    void addSphereColliderGeometry(SphereCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);

    bool rebuildTileCacheTiles(NavMesh *navMesh);

protected:
    std::unordered_map<Scene *, NavMesh *> m_sceneNavMeshes;

    std::vector<AgentType> m_agentTypes;

    mutable std::mutex m_dataMutex;

    dtTileCache *m_tileCache = nullptr;
    dtTileCacheCompressor *m_tileCompressor = nullptr;

};

#endif // NAVIGATIONSYSTEM_H
