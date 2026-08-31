#ifndef NAVIGATIONSURFACE_H
#define NAVIGATIONSURFACE_H

#include <component.h>
#include <future>

class NavMesh;
class NavigationSystem;

enum SurfaceGeometrySource {
    AllColliders,
    CollidersWithTag
};

class NavigationSurface : public Component {
    A_OBJECT(NavigationSurface, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(bool, autoBuild, NavigationSurface::autoBuild, NavigationSurface::setAutoBuild),
        A_PROPERTY(Vector3, origin, NavigationSurface::origin, NavigationSurface::setOrigin),
        A_PROPERTY(Vector3, extents, NavigationSurface::extents, NavigationSurface::setExtents),
        A_PROPERTY(float, cellSize, NavigationSurface::cellSize, NavigationSurface::setCellSize),
        A_PROPERTY(float, cellHeight, NavigationSurface::cellHeight, NavigationSurface::setCellHeight),
        A_PROPERTY(int, tileSize, NavigationSurface::tileSize, NavigationSurface::setTileSize),
        A_PROPERTY(int, agentType, NavigationSurface::agentType, NavigationSurface::setAgentType),
        A_PROPERTY(int, geometrySource, NavigationSurface::geometrySource, NavigationSurface::setGeometrySource),
        A_PROPERTY(TString, includeTag, NavigationSurface::includeTag, NavigationSurface::setIncludeTag)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    NavigationSurface();
    ~NavigationSurface();

    bool autoBuild() const { return m_autoBuild; }
    void setAutoBuild(bool autoBuild);

    Vector3 origin() const { return m_origin; }
    void setOrigin(const Vector3 &origin);

    Vector3 extents() const { return m_extents; }
    void setExtents(const Vector3 &extents);

    float cellSize() const { return m_cellSize; }
    void setCellSize(float size);

    float cellHeight() const { return m_cellHeight; }
    void setCellHeight(float height);

    int tileSize() const { return m_tileSize; }
    void setTileSize(int tileSize);

    int agentType() const { return m_agentType; }
    void setAgentType(int type);

    int geometrySource() const { return static_cast<int>(m_geometrySource); }
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

    void drawGizmos() override;

protected:
    bool m_autoBuild = true;
    Vector3 m_origin = Vector3(0, 0, 0);
    Vector3 m_extents = Vector3(100, 100, 100);
    float m_cellSize = 0.3f;
    float m_cellHeight = 0.2f;
    int m_tileSize = 64;
    int m_agentType = 0;
    SurfaceGeometrySource m_geometrySource = SurfaceGeometrySource::AllColliders;
    TString m_includeTag;

    NavMesh *m_navMesh = nullptr;
};

#endif // NAVIGATIONSURFACE_H
