#ifndef NAVMESHLINK_H
#define NAVMESHLINK_H

#include <component.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavMeshLink : public Component {
    A_OBJECT(NavMeshLink, Component, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTYEX(int, agentType, NavMeshLink::agentType, NavMeshLink::setAgentType, "editor=AgentTypeEdit"),
        A_PROPERTY(Vector3, startPoint, NavMeshLink::startPoint, NavMeshLink::setStartPoint),
        A_PROPERTY(Vector3, endPoint, NavMeshLink::endPoint, NavMeshLink::setEndPoint),
        A_PROPERTY(bool, bidirectional, NavMeshLink::isBidirectional, NavMeshLink::setBidirectional)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    NavMeshLink();
    ~NavMeshLink();

    int agentType() const;
    void setAgentType(int type);

    int areaType() const;
    void setAreaType(int type);

    Vector3 startPoint() const;
    void setStartPoint(const Vector3 &point);

    Vector3 endPoint() const;
    void setEndPoint(const Vector3 &point);

    bool isBidirectional() const;
    void setBidirectional(bool bidirectional);

protected:
    void drawGizmos() override;
    void drawGizmosSelected() override;

protected:
    Vector3 m_startPoint = Vector3(-2.0f, 0.0f, 0.0f);
    Vector3 m_endPoint = Vector3(2.0f, 0.0f, 0.0f);

    int m_agentType = 0;
    int m_areaType = 1;

    bool m_bidirectional = true;

};

#endif // NAVMESHLINK_H
