#ifndef NAVIGATIONLINK_H
#define NAVIGATIONLINK_H

#include <component.h>

class NavigationLink : public Component {
    A_OBJECT(NavigationLink, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(Vector3, startPoint, NavigationLink::startPoint, NavigationLink::setStartPoint),
        A_PROPERTY(Vector3, endPoint, NavigationLink::endPoint, NavigationLink::setEndPoint),
        A_PROPERTY(float, radius, NavigationLink::radius, NavigationLink::setRadius),
        A_PROPERTY(bool, bidirectional, NavigationLink::isBidirectional, NavigationLink::setBidirectional)
    )
    A_NOMETHODS()
    A_NOENUMS()

    enum LinkDirection {
        Bidirectional,
        Forward,
        Backward
    };

public:
    NavigationLink();
    ~NavigationLink();

    void composeComponent() override;

    Vector3 startPoint() const { return m_startPoint; }
    void setStartPoint(const Vector3 &point);

    Vector3 endPoint() const { return m_endPoint; }
    void setEndPoint(const Vector3 &point);

    float radius() const { return m_radius; }
    void setRadius(float radius);

    bool isBidirectional() const { return m_direction == LinkDirection::Bidirectional; }
    void setBidirectional(bool bidirectional);

    LinkDirection direction() const { return m_direction; }
    void setDirection(LinkDirection direction);

protected:
    Vector3 m_startPoint;
    Vector3 m_endPoint;
    float m_radius = 0.6f;
    LinkDirection m_direction = LinkDirection::Bidirectional;

};

#endif // NAVIGATIONLINK_H
