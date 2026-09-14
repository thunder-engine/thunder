/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
    TString name = "Humanoid";
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

    static int agentTypeCount();
    static AgentType agentType(int index);
    static void setAgentType(int index, const AgentType &type);

    static unsigned short agentFlag(int index) {
        if(index < 0 || index >= 16) {
            return 0x0001;
        }
        return static_cast<unsigned short>(1u << index);
    }

    uint32_t addObstacle(NavMeshObstacle &obstacle);
    bool removeObstacle(uint32_t obstacleId);

protected:
    std::vector<Vector3> findPathOnNavMesh(NavMesh *navMesh, const Vector3 &start, const Vector3 &end, int agentType);

    bool rebuildTileCacheTiles(NavMesh *navMesh);

protected:
    std::unordered_map<NavMeshSurface *, NavMesh *> m_surfaceNavMeshes;

    static std::vector<AgentType> s_agentTypes;

    mutable std::mutex m_dataMutex;

    dtTileCache *m_tileCache = nullptr;
    dtTileCacheCompressor *m_tileCompressor = nullptr;

};

#endif // NAVIGATIONSYSTEM_H
