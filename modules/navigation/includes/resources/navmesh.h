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
#ifndef NAVMESH_H
#define NAVMESH_H

#include <resource.h>
#include <navigation.h>

class dtNavMesh;
class dtNavMeshQuery;

typedef uint32_t dtTileRef;

class NAVIGATION_EXPORT NavMesh : public Resource {
    A_OBJECT(NavMesh, Resource, Resources)

public:
    NavMesh();
    ~NavMesh();

    dtNavMesh *navMesh() const;
    dtNavMeshQuery *query() const;

    dtTileRef tileRef(int tileX, int tileY) const;

    bool setData(const ByteArray &data);

protected:
    void cleanup();

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

protected:
    dtNavMesh *m_navMesh;
    dtNavMeshQuery *m_query;

    dtTileRef m_tileRef;
};

#endif // NAVMESH_H
