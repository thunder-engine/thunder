/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
