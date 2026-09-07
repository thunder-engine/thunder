#ifndef NAVIGATIONSYSTEM_H
#define NAVIGATIONSYSTEM_H

#include <system.h>

#include <DetourNavMesh.h>

#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>

class Scene;
class NavMesh;
class NavMeshObstacle;
class NavMeshLink;
class NavMeshAgent;
class NavMeshSurface;

class BoxCollider;
class MeshCollider;
class CapsuleCollider;
class SphereCollider;

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

    std::vector<Vector3> findPath(NavMeshAgent &agent);

    bool registerNavMesh(NavMeshSurface &surface);
    void unregisterNavMesh(NavMeshSurface &surface);

    NavMesh *findNavMeshAtPosition(const Vector3 &position, int agentType) const;

    bool addObstacle(NavMeshObstacle &obstacle);
    bool removeObstacle(uint32_t obstacleId);

    bool buildNavMeshFromSurface(NavMeshSurface &surface);

    AgentType agentType(int index) const;

protected:
    bool buildNavMeshData(NavMeshSurface &surface, const Vector3Vector &vertices, const std::vector<int> &indices);

    std::vector<Vector3> findPathOnNavMesh(NavMesh *navMesh, const Vector3 &start, const Vector3 &end, int agentType);

    bool rebuildTileCacheTiles(NavMesh *navMesh);

protected:
    std::unordered_map<NavMeshSurface *, NavMesh *> m_surfaceNavMeshes;

    std::vector<AgentType> m_agentTypes;

    mutable std::mutex m_dataMutex;

    dtTileCache *m_tileCache = nullptr;
    dtTileCacheCompressor *m_tileCompressor = nullptr;

};

#endif // NAVIGATIONSYSTEM_H
