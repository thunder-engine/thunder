#ifndef NAVIGATIONSYSTEM_H
#define NAVIGATIONSYSTEM_H

#include <system.h>

#include <DetourNavMesh.h>
#include <DetourTileCache.h>
#include <DetourTileCacheBuilder.h>

class NavMesh;
class NavMeshObstacle;
class NavMeshAgent;
class NavMeshSurface;

struct rcCompactHeightfield;
struct rcConfig;

struct AgentType {
    TString name;
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

    int threadPolicy() const override { return Pool; }

    std::vector<Vector3> findPath(NavMeshAgent &agent, const Vector3 &target);

    bool registerNavMesh(NavMeshSurface &surface);
    void unregisterNavMesh(NavMeshSurface &surface);

    NavMesh *findNavMeshAtPosition(const Vector3 &position, int agentType) const;

    uint32_t addObstacle(NavMeshObstacle &obstacle);
    bool removeObstacle(uint32_t obstacleId);

    AgentType agentType(int index) const;

protected:
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
