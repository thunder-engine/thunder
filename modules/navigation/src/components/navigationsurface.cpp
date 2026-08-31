#include "components/navigationsurface.h"

#include "navigationsystem.h"
#include <actor.h>
#include <scene.h>
#include <transform.h>
#include <engine.h>
#include <log.h>

#include "resources/navmesh.h"

NavigationSurface::NavigationSurface() :
        Component() {
    PROFILE_FUNCTION();

    static uint32_t hash = Mathf::hashString("navsurf");
    addTagByHash(hash);
}

NavigationSurface::~NavigationSurface() {
    PROFILE_FUNCTION();

    if(m_navMesh) {
        m_navMesh->decRef();
        m_navMesh = nullptr;
    }
}

void NavigationSurface::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    Component::loadUserData(data);

    auto it = data.find("navMesh");
    if(it != data.end()) {
        TString ref = it->second.toString();
        setNavMesh(Engine::loadResource<NavMesh>(ref));
    }
}

VariantMap NavigationSurface::saveUserData() const {
    PROFILE_FUNCTION();

    VariantMap result = Component::saveUserData();
    if(m_navMesh) {
        result["navMesh"] = Engine::reference(m_navMesh);
    }

    return result;
}

void NavigationSurface::setAutoBuild(bool autoBuild) {
    if(m_autoBuild != autoBuild) {
        m_autoBuild = autoBuild;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setOrigin(const Vector3 &origin) {
    if(m_origin != origin) {
        m_origin = origin;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setExtents(const Vector3 &extents) {
    if(m_extents != extents) {
        m_extents = extents;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setCellSize(float size) {
    if(size <= 0.0f) {
        aWarning() << "NavigationSurface: Cell size must be positive";
        return;
    }

    if(m_cellSize != size) {
        m_cellSize = size;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setCellHeight(float height) {
    if(height <= 0.0f) {
        aWarning() << "NavigationSurface: Cell height must be positive";
        return;
    }

    if(m_cellHeight != height) {
        m_cellHeight = height;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setTileSize(int tileSize) {
    if(tileSize <= 0) {
        aWarning() << "NavigationSurface: Tile size must be positive";
        return;
    }

    if(m_tileSize != tileSize) {
        m_tileSize = tileSize;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setAgentType(int type) {
    if(m_agentType != type) {
        m_agentType = type;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setGeometrySource(int source) {
    SurfaceGeometrySource newSource = static_cast<SurfaceGeometrySource>(source);
    if(m_geometrySource != newSource) {
        m_geometrySource = newSource;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setIncludeTag(const TString &tag) {
    if(m_includeTag != tag) {
        m_includeTag = tag;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavigationSurface::setNavMesh(NavMesh *navMesh) {
    if(m_navMesh != navMesh) {
        m_navMesh = navMesh;
        onNavMeshChanged();
    }
}

void NavigationSurface::onNavMeshChanged() {
    if(m_navMesh && m_navMesh->state() == NavMesh::Ready) {
        NavigationSystem *navSystem = dynamic_cast<NavigationSystem *>(system());
        if(navSystem) {
            navSystem->registerNavMesh(scene(), m_navMesh);
        }
    }
}

bool NavigationSurface::build() {
    PROFILE_FUNCTION();

    NavigationSystem *navSystem = dynamic_cast<NavigationSystem *>(system());
    if(navSystem) {
        if(!navSystem->buildNavMeshFromSurface(this)) {
            aError() << "NavigationSurface: Failed to build NavMesh";
            return false;
        }

        m_navMesh = navSystem->getNavMesh(scene());
    }

    return m_navMesh != nullptr && m_navMesh->state() == NavMesh::Ready;
}

std::future<bool> NavigationSurface::buildAsync() {
    PROFILE_FUNCTION();

    return std::async(std::launch::async, [this]() -> bool {
        return build();
    });
}

void NavigationSurface::clear() {
    PROFILE_FUNCTION();

    NavigationSystem *navSystem = dynamic_cast<NavigationSystem *>(system());
    if(navSystem) {
        navSystem->unregisterNavMesh(scene());
    }

    if(m_navMesh) {
        m_navMesh->decRef();
        m_navMesh = nullptr;
    }
}

void NavigationSurface::drawGizmos() {
    if(m_navMesh) {

    }
}
