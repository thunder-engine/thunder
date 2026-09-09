#include "components/navmeshlink.h"

#include <transform.h>
#include <gizmos.h>

#include "navigationsystem.h"

NavMeshLink::NavMeshLink() :
        Component(),
        m_startPoint(Vector3(-2.0f, 0.0f, 0.0f)),
        m_endPoint(Vector3(2.0f, 0.0f, 0.0f)),
        m_agentType(0),
        m_areaType(1),
        m_bidirectional(true) {
    PROFILE_FUNCTION();

    static uint32_t hash = Mathf::hashString("navmeshlink");
    addTagByHash(hash);
}

NavMeshLink::~NavMeshLink() {
    PROFILE_FUNCTION();
}

int NavMeshLink::agentType() const {
    return m_agentType;
}

void NavMeshLink::setAgentType(int type) {
    m_agentType = type;
}

int NavMeshLink::areaType() const {
    return m_areaType;
}

void NavMeshLink::setAreaType(int type) {
    m_areaType = type;
}

Vector3 NavMeshLink::startPoint() const {
    return m_startPoint;
}

void NavMeshLink::setStartPoint(const Vector3 &point) {
    m_startPoint = point;
}

Vector3 NavMeshLink::endPoint() const {
    return m_endPoint;
}

void NavMeshLink::setEndPoint(const Vector3 &point) {
    m_endPoint = point;
}

bool NavMeshLink::isBidirectional() const {
    return m_bidirectional;
}

void NavMeshLink::setBidirectional(bool bidirectional) {
    m_bidirectional = bidirectional;
}

void NavMeshLink::drawGizmos() {
    Matrix4 worldMatrix = transform()->worldTransform();

    Gizmos::drawLines({worldMatrix * m_startPoint, worldMatrix * m_endPoint}, {0, 1}, Vector4(0.0f, 1.0f, 1.0f, 0.5f));
}

void NavMeshLink::drawGizmosSelected() {
    Matrix4 worldMatrix = transform()->worldTransform();

    float radius = static_cast<NavigationSystem *>(system())->agentType(m_agentType).radius;

    Gizmos::drawSolidSphere(worldMatrix * m_startPoint, radius, Vector4(0.0f, 1.0f, 1.0f, 0.5f));
    Gizmos::drawSolidSphere(worldMatrix * m_endPoint, radius, Vector4(0.0f, 1.0f, 1.0f, 0.5f));
}
