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
