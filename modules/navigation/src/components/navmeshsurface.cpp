#include "components/navmeshsurface.h"

#include <actor.h>
#include <scene.h>
#include <transform.h>
#include <engine.h>
#include <log.h>

#include <boxcollider.h>
#include <spherecollider.h>
#include <capsulecollider.h>
#include <meshcollider.h>

#include "components/navmeshobstacle.h"

#include "resources/navmesh.h"
#include "utils/debugrender.h"

#include "navigationsystem.h"

NavMeshSurface::NavMeshSurface() :
        Component(),
        m_navMesh(nullptr),
        m_tileSize(256),
        m_agentType(0),
        m_geometrySource(AllColliders),
        m_autoBuild(true) {
    PROFILE_FUNCTION();

    static uint32_t hash = Mathf::hashString("navsurf");
    addTagByHash(hash);
}

NavMeshSurface::~NavMeshSurface() {
    PROFILE_FUNCTION();

    if(m_navMesh) {
        m_navMesh->decRef();
        m_navMesh = nullptr;
    }
}

void NavMeshSurface::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    Component::loadUserData(data);

    auto it = data.find("navMesh");
    if(it != data.end()) {
        TString ref = it->second.toString();
        setNavMesh(Engine::loadResource<NavMesh>(ref));
    }
}

VariantMap NavMeshSurface::saveUserData() const {
    PROFILE_FUNCTION();

    VariantMap result = Component::saveUserData();
    if(m_navMesh) {
        result["navMesh"] = Engine::reference(m_navMesh);
    }

    return result;
}

void NavMeshSurface::setAutoBuild(bool autoBuild) {
    if(m_autoBuild != autoBuild) {
        m_autoBuild = autoBuild;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavMeshSurface::setTileSize(int tileSize) {
    if(tileSize <= 0) {
        aWarning() << "NavMeshSurface: Tile size must be positive";
        return;
    }

    if(m_tileSize != tileSize) {
        m_tileSize = tileSize;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavMeshSurface::setAgentType(int type) {
    if(m_agentType != type) {
        m_agentType = type;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavMeshSurface::setGeometrySource(int source) {
    SurfaceGeometrySource newSource = static_cast<SurfaceGeometrySource>(source);
    if(m_geometrySource != newSource) {
        m_geometrySource = newSource;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavMeshSurface::setIncludeTag(const TString &tag) {
    if(m_includeTag != tag) {
        m_includeTag = tag;
        if(m_autoBuild) {
            build();
        }
    }
}

void NavMeshSurface::setNavMesh(NavMesh *navMesh) {
    if(m_navMesh != navMesh) {
        m_navMesh = navMesh;
        onNavMeshChanged();
    }
}

void NavMeshSurface::onNavMeshChanged() {
    if(m_navMesh && m_navMesh->state() == NavMesh::Ready) {
        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        navSystem->registerNavMesh(*this);
    }
}

bool NavMeshSurface::build() {
    PROFILE_FUNCTION();

    NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
    if(!navSystem->buildNavMeshFromSurface(*this)) {
        aError() << "NavMeshSurface: Failed to build NavMesh";
        return false;
    }

    return m_navMesh != nullptr && m_navMesh->state() == NavMesh::Ready;
}

std::future<bool> NavMeshSurface::buildAsync() {
    PROFILE_FUNCTION();

    return std::async(std::launch::async, [this]() -> bool {
        return build();
    });
}

void NavMeshSurface::clear() {
    PROFILE_FUNCTION();

    NavigationSystem *navSystem = dynamic_cast<NavigationSystem *>(system());
    if(navSystem) {
        navSystem->unregisterNavMesh(*this);
    }

    if(m_navMesh) {
        m_navMesh->decRef();
        m_navMesh = nullptr;
    }
}

void NavMeshSurface::drawGizmosSelected() {
    if(m_navMesh) {
        DebugRender render;
        render.draw(m_navMesh->navMesh());
    }
}

bool NavMeshSurface::collectGeometry(Vector3Vector &outVertices, std::vector<int> &outIndices) {
    static uint32_t hash = Mathf::hashString("collider");

    Scene *scene = NavMeshSurface::scene();
    for(auto it : scene->getObjectsInGroupByHash(hash)) {
        Collider *collider = dynamic_cast<Collider *>(it);
        Actor *actor = collider->actor();
        if(!actor || !actor->isEnabled()) continue;
        if(actor->getComponent<NavMeshObstacle>() != nullptr) continue;

        BoxCollider *boxCollider = dynamic_cast<BoxCollider *>(collider);
        if(boxCollider && boxCollider->isEnabled()) {
            addBoxColliderGeometry(boxCollider, outVertices, outIndices);
            continue;
        }

        MeshCollider *meshCollider = dynamic_cast<MeshCollider *>(collider);
        if(meshCollider && meshCollider->isEnabled()) {
            addMeshColliderGeometry(meshCollider, outVertices, outIndices);
            continue;
        }

        CapsuleCollider *capsuleCollider = dynamic_cast<CapsuleCollider *>(collider);
        if(capsuleCollider && capsuleCollider->isEnabled()) {
            addCapsuleColliderGeometry(capsuleCollider, outVertices, outIndices);
            continue;
        }

        SphereCollider *sphereCollider = dynamic_cast<SphereCollider *>(collider);
        if(sphereCollider && sphereCollider->isEnabled()) {
            addSphereColliderGeometry(sphereCollider, outVertices, outIndices);
            continue;
        }
    }

    return !outVertices.empty() && !outIndices.empty();
}


void NavMeshSurface::addBoxColliderGeometry(BoxCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    Vector3 size = collider->size();
    Vector3 center = collider->center();
    Transform *transform = collider->transform();

    Vector3 localVerts[8] = {
        Vector3(-size.x * 0.5f, -size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f, -size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f, -size.y * 0.5f,  size.z * 0.5f),
        Vector3(-size.x * 0.5f, -size.y * 0.5f,  size.z * 0.5f),
        Vector3(-size.x * 0.5f,  size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f,  size.y * 0.5f, -size.z * 0.5f),
        Vector3( size.x * 0.5f,  size.y * 0.5f,  size.z * 0.5f),
        Vector3(-size.x * 0.5f,  size.y * 0.5f,  size.z * 0.5f)
    };

    size_t startIndex = outVertices.size();

    for(int i = 0; i < 8; ++i) {
        Vector3 worldPos = center + localVerts[i];
        outVertices.push_back(transform->worldTransform() * worldPos);
    }

    static const uint32_t indices[] = {
        0, 1, 2,  0, 2, 3,
        4, 6, 5,  4, 7, 6,
        0, 4, 5,  0, 5, 1,
        1, 5, 6,  1, 6, 2,
        2, 6, 7,  2, 7, 3,
        3, 7, 4,  3, 4, 0
    };

    for(uint32_t idx : indices) {
        outIndices.push_back(static_cast<uint32_t>(startIndex + idx));
    }
}

void NavMeshSurface::addMeshColliderGeometry(MeshCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    Mesh *mesh = collider->mesh();
    if(!mesh) {
        return;
    }

    Transform *transform = collider->transform();

    size_t startIndex = outVertices.size();

    const auto &localVerts = mesh->vertices();
    for(const Vector3 &v : localVerts) {
        outVertices.push_back(transform->worldTransform() * v);
    }

    const auto &localIndices = mesh->indices();
    for(uint32_t idx : localIndices) {
        outIndices.push_back(static_cast<uint32_t>(startIndex + idx));
    }
}

void NavMeshSurface::addCapsuleColliderGeometry(CapsuleCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    /// \todo: implement this
}

void NavMeshSurface::addSphereColliderGeometry(SphereCollider *collider, Vector3Vector &outVertices, std::vector<int> &outIndices) {
    /// \todo: implement this
}

