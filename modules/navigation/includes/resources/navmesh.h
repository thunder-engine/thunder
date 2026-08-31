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

    dtNavMesh *navMesh() const { return m_navMesh; }
    dtNavMeshQuery *query() const { return m_query; }

    float cellSize() const { return m_cellSize; }
    void setCellSize(float size);
    float cellHeight() const { return m_cellHeight; }
    void setCellHeight(float height);
    int tileSize() const { return m_tileSize; }
    void setTileSize(int size);
    Vector3 origin() const { return m_origin; }
    void setOrigin(const Vector3 &origin);

    dtTileRef tileRef(int tileX, int tileY) const;

    bool setData(ByteArray &data);

protected:
    bool initNavMesh();
    void cleanup();

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

protected:
    dtNavMesh *m_navMesh = nullptr;
    dtNavMeshQuery *m_query = nullptr;

    Vector3 m_origin = Vector3(0, 0, 0);
    float m_cellSize = 0.3f;
    float m_cellHeight = 0.2f;
    int m_tileSize = 64;
};

#endif // NAVMESH_H
