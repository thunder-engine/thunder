#ifndef NAVIGATIONLINK_H
#define NAVIGATIONLINK_H

#include <component.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavigationLink : public Component {
    A_OBJECT(NavigationLink, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(Vector3, startPoint, NavigationLink::startPoint, NavigationLink::setStartPoint),
        A_PROPERTY(Vector3, endPoint, NavigationLink::endPoint, NavigationLink::setEndPoint),
        A_PROPERTY(float, width, NavigationLink::width, NavigationLink::setWidth),
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

    Vector3 startPoint() const { return m_startPoint; }
    void setStartPoint(const Vector3 &point);

    Vector3 endPoint() const { return m_endPoint; }
    void setEndPoint(const Vector3 &point);

    float width() const { return m_width; }
    void setWidth(float width);

    bool isBidirectional() const { return m_direction == LinkDirection::Bidirectional; }
    void setBidirectional(bool bidirectional);

    LinkDirection direction() const { return m_direction; }
    void setDirection(LinkDirection direction);

protected:
    void drawGizmosSelected() override;

protected:
    Vector3 m_startPoint = Vector3(-2.0f, 0.0f, 0.0f);
    Vector3 m_endPoint = Vector3(2.0f, 0.0f, 0.0f);

    float m_width = 0.0f;

    LinkDirection m_direction = LinkDirection::Bidirectional;

};

#endif // NAVIGATIONLINK_H
