#include "components/navmeshlink.h"

#include <transform.h>
#include <gizmos.h>

#include "navigationsystem.h"

/*!
    \class NavMeshLink
    \brief The NavMeshLink class connects two points of a navigation mesh.
    \inmodule Navigation

    A navigation link defines a traversable connection between startPoint()
    and endPoint(). The link can be restricted to a configured agent type and
    can allow traversal in one or both directions.

    Links are useful for navigation routes that are not represented by the
    regular navigation mesh, such as jumps, doors, or teleport connections.

    \sa NavMeshAgent, NavigationSystem
*/

/*!
    Constructs a navigation link with default endpoints and agent settings.
*/
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

/*!
    Destroys the navigation link.
*/
NavMeshLink::~NavMeshLink() {
    PROFILE_FUNCTION();
}

/*!
    Returns the navigation mesh agent type allowed to use this link.
*/
int NavMeshLink::agentType() const {
    return m_agentType;
}

/*!
    Sets the navigation mesh agent \a type allowed to use this link.
*/
void NavMeshLink::setAgentType(int type) {
    m_agentType = type;
}

/*!
    Returns the navigation area type assigned to this link.
*/
int NavMeshLink::areaType() const {
    return m_areaType;
}

/*!
    Sets the navigation area \a type assigned to this link.
*/
void NavMeshLink::setAreaType(int type) {
    m_areaType = type;
}

/*!
    Returns the local-space start point of the link.
*/
Vector3 NavMeshLink::startPoint() const {
    return m_startPoint;
}

/*!
    Sets the local-space start \a point of the link.
*/
void NavMeshLink::setStartPoint(const Vector3 &point) {
    m_startPoint = point;
}

/*!
    Returns the local-space end point of the link.
*/
Vector3 NavMeshLink::endPoint() const {
    return m_endPoint;
}

/*!
    Sets the local-space end \a point of the link.
*/
void NavMeshLink::setEndPoint(const Vector3 &point) {
    m_endPoint = point;
}

/*!
    Returns true when the link can be traversed in both directions.
*/
bool NavMeshLink::isBidirectional() const {
    return m_bidirectional;
}

/*!
    Enables or disables traversal from endPoint() back to startPoint().

    When \a bidirectional is false, the link can only be traversed from
    startPoint() to endPoint().
*/
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
