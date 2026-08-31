#include "components/navigationlink.h"

#include <transform.h>

NavigationLink::NavigationLink() :
        Component() {
    PROFILE_FUNCTION();

    static uint32_t hash = Mathf::hashString("navigationlink");
    addTagByHash(hash);
}

NavigationLink::~NavigationLink() {
    PROFILE_FUNCTION();
}

void NavigationLink::composeComponent() {
    PROFILE_FUNCTION();

    Vector3 pos = transform()->position();
    m_startPoint = pos + Vector3(-2.0f, 0.0f, 0.0f);
    m_endPoint = pos + Vector3(2.0f, 0.0f, 0.0f);
}

void NavigationLink::setStartPoint(const Vector3 &point) {
    m_startPoint = point;
}

void NavigationLink::setEndPoint(const Vector3 &point) {
    m_endPoint = point;
}

void NavigationLink::setRadius(float radius) {
    m_radius = radius;
}

void NavigationLink::setBidirectional(bool bidirectional) {
    setDirection(bidirectional ? LinkDirection::Bidirectional : LinkDirection::Forward);
}

void NavigationLink::setDirection(LinkDirection direction) {
    m_direction = direction;
}
