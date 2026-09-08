#ifndef NAVMESHSURFACE_H
#define NAVMESHSURFACE_H

#include <component.h>
#include <future>

#include <navigation.h>

class NavMesh;

class BoxCollider;
class MeshCollider;
class CapsuleCollider;
class SphereCollider;

class NAVIGATION_EXPORT NavMeshSurface : public Component {
    A_OBJECT(NavMeshSurface, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(bool, autoBuild, NavMeshSurface::autoBuild, NavMeshSurface::setAutoBuild),
        A_PROPERTY(int, tileSize, NavMeshSurface::tileSize, NavMeshSurface::setTileSize),
        A_PROPERTY(int, agentType, NavMeshSurface::agentType, NavMeshSurface::setAgentType),
        A_PROPERTY(int, geometrySource, NavMeshSurface::geometrySource, NavMeshSurface::setGeometrySource),
        A_PROPERTY(TString, includeTag, NavMeshSurface::includeTag, NavMeshSurface::setIncludeTag)
    )
    A_METHODS(
        A_METHOD(bool, NavMeshSurface::build),
        A_METHOD(void, NavMeshSurface::clear)
    )
    A_NOENUMS()

public:
    enum SurfaceGeometrySource {
        AllColliders,
        CollidersWithTag
    };

public:
    NavMeshSurface();
    ~NavMeshSurface();

    bool autoBuild() const { return m_autoBuild; }
    void setAutoBuild(bool autoBuild);

    int tileSize() const { return m_tileSize; }
    void setTileSize(int tileSize);

    int agentType() const { return m_agentType; }
    void setAgentType(int type);

    int geometrySource() const { return m_geometrySource; }
    void setGeometrySource(int source);

    TString includeTag() const { return m_includeTag; }
    void setIncludeTag(const TString &tag);

    NavMesh *navMesh() const { return m_navMesh; }
    void setNavMesh(NavMesh *navMesh);

    bool build();
    std::future<bool> buildAsync();
    void clear();

protected:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void onNavMeshChanged();

    void drawGizmosSelected() override;

    bool collectGeometry(Vector3Vector &outVertices, std::vector<int> &outIndices);

    void addBoxColliderGeometry(BoxCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);
    void addMeshColliderGeometry(MeshCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);
    void addCapsuleColliderGeometry(CapsuleCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);
    void addSphereColliderGeometry(SphereCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices);

    bool buildNavMeshData(const Vector3Vector &vertices, const std::vector<int> &indices);

protected:
    friend class NavigationSystem;

    TString m_includeTag;

    NavMesh *m_navMesh;

    int m_tileSize;
    int m_agentType;
    int m_geometrySource;

    bool m_autoBuild;
};

#endif // NAVMESHSURFACE_H
