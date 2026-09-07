#ifndef NAVMESHLINK_H
#define NAVMESHLINK_H

#include <component.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavMeshLink : public Component {
    A_OBJECT(NavMeshLink, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(Vector3, startPoint, NavMeshLink::startPoint, NavMeshLink::setStartPoint),
        A_PROPERTY(Vector3, endPoint, NavMeshLink::endPoint, NavMeshLink::setEndPoint),
        A_PROPERTY(bool, bidirectional, NavMeshLink::isBidirectional, NavMeshLink::setBidirectional)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    NavMeshLink();
    ~NavMeshLink();

    Vector3 startPoint() const { return m_startPoint; }
    void setStartPoint(const Vector3 &point);

    Vector3 endPoint() const { return m_endPoint; }
    void setEndPoint(const Vector3 &point);

    bool isBidirectional() const { return m_bidirectional; }
    void setBidirectional(bool bidirectional);

protected:
    void drawGizmosSelected() override;

protected:
    Vector3 m_startPoint = Vector3(-2.0f, 0.0f, 0.0f);
    Vector3 m_endPoint = Vector3(2.0f, 0.0f, 0.0f);

    float m_width = 0.0f;

    bool m_bidirectional = true;

};

#endif // NAVMESHLINK_H
