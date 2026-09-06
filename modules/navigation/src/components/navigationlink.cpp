#include "components/navigationlink.h"

#include <transform.h>
#include <gizmos.h>

NavigationLink::NavigationLink() :
        Component() {
    PROFILE_FUNCTION();

    static uint32_t hash = Mathf::hashString("navigationlink");
    addTagByHash(hash);
}

NavigationLink::~NavigationLink() {
    PROFILE_FUNCTION();
}

void NavigationLink::setStartPoint(const Vector3 &point) {
    m_startPoint = point;
}

void NavigationLink::setEndPoint(const Vector3 &point) {
    m_endPoint = point;
}

void NavigationLink::setWidth(float width) {
    m_width = width;
}

void NavigationLink::setBidirectional(bool bidirectional) {
    setDirection(bidirectional ? LinkDirection::Bidirectional : LinkDirection::Forward);
}

void NavigationLink::setDirection(LinkDirection direction) {
    m_direction = direction;
}

void NavigationLink::drawGizmosSelected() {
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
