#ifndef NAVIGATIONSURFACE_H
#define NAVIGATIONSURFACE_H

#include <component.h>
#include <future>

#include <navigation.h>

class NavMesh;

class NAVIGATION_EXPORT NavigationSurface : public Component {
    A_OBJECT(NavigationSurface, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(bool, autoBuild, NavigationSurface::autoBuild, NavigationSurface::setAutoBuild),
        A_PROPERTY(int, tileSize, NavigationSurface::tileSize, NavigationSurface::setTileSize),
        A_PROPERTY(int, agentType, NavigationSurface::agentType, NavigationSurface::setAgentType),
        A_PROPERTY(int, geometrySource, NavigationSurface::geometrySource, NavigationSurface::setGeometrySource),
        A_PROPERTY(TString, includeTag, NavigationSurface::includeTag, NavigationSurface::setIncludeTag)
    )
    A_METHODS(
        A_METHOD(bool, NavigationSurface::build),
        A_METHOD(void, NavigationSurface::clear)
    )
    A_NOENUMS()

public:
    enum SurfaceGeometrySource {
        AllColliders,
        CollidersWithTag
    };

public:
    NavigationSurface();
    ~NavigationSurface();

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

protected:
    TString m_includeTag;

    NavMesh *m_navMesh;

    int m_tileSize;
    int m_agentType;
    int m_geometrySource;

    bool m_autoBuild;
};

#endif // NAVIGATIONSURFACE_H
