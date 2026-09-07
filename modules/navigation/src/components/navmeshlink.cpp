#include "components/navmeshlink.h"

#include <transform.h>
#include <gizmos.h>

NavMeshLink::NavMeshLink() :
        Component() {
    PROFILE_FUNCTION();

    static uint32_t hash = Mathf::hashString("navmeshlink");
    addTagByHash(hash);
}

NavMeshLink::~NavMeshLink() {
    PROFILE_FUNCTION();
}

void NavMeshLink::setStartPoint(const Vector3 &point) {
    m_startPoint = point;
}

void NavMeshLink::setEndPoint(const Vector3 &point) {
    m_endPoint = point;
}

void NavMeshLink::setBidirectional(bool bidirectional) {
    m_bidirectional = bidirectional;
}

void NavMeshLink::drawGizmosSelected() {
    Matrix4 worldMatrix = transform()->worldTransform();

    Vector3 worldStart = worldMatrix * m_startPoint;
    Vector3 worldEnd = worldMatrix * m_endPoint;

    Vector3 dir = worldEnd - worldStart;
    float length = dir.normalize();
    if(length < 0.001f) {
        return;
    }

    Vector3 right = dir.cross(Vector3(0.0f, 1.0f, 0.0f));
    right.normalize();

    Vector3Vector vertices = {
        worldStart + right * m_width * 0.5f,
        worldStart - right * m_width * 0.5f,
        worldEnd + right * m_width * 0.5f,
        worldEnd - right * m_width * 0.5f
    };

    Gizmos::drawLines(vertices, {0, 1, 1, 3, 3, 2, 2, 0}, Vector4(0.0f, 1.0f, 1.0f, 0.7f));
}
