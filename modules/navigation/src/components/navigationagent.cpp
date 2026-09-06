#include "components/navigationagent.h"

#include "navigationsystem.h"

#include <transform.h>
#include <timer.h>
#include <gizmos.h>

#include <log.h>

NavigationAgent::NavigationAgent() :
    NativeBehaviour() {
    PROFILE_FUNCTION();
}

NavigationAgent::~NavigationAgent() {
    PROFILE_FUNCTION();

    stop();
}

int NavigationAgent::agentType() const {
    return m_agentType;
}

void NavigationAgent::setAgentType(int type) {
    m_agentType = type;
}

float NavigationAgent::speed() const {
    return m_maxSpeed;
}

void NavigationAgent::setSpeed(float speed) {
    m_maxSpeed = speed;
}

float NavigationAgent::angularSpeed() const {
    return m_angularSpeed;
}

void NavigationAgent::setAngularSpeed(float angularSpeed) {
    m_angularSpeed = angularSpeed;
}

float NavigationAgent::acceleration() const {
    return m_maxAcceleration;
}

void NavigationAgent::setAcceleration(float acceleration) {
    m_maxAcceleration = acceleration;
}

float NavigationAgent::stoppingDistance() const {
    return m_stoppingDistance;
}

void NavigationAgent::setStoppingDistance(float stoppingDistance) {
    m_stoppingDistance = stoppingDistance;
}

bool NavigationAgent::autoBraking() const {
    return m_autoBraking;
}

void NavigationAgent::setAutoBraking(bool autoBraking) {
    m_autoBraking = autoBraking;
}

bool NavigationAgent::autoRepath() const {
    return m_autoRepath;
}

void NavigationAgent::setAutoRepath(bool autoRepath) {
    m_autoRepath = autoRepath;
}

bool NavigationAgent::moveTo(const Vector3 &target) {
    PROFILE_FUNCTION();

    m_target = target;
    m_hasTarget = true;
    m_state = NavigationState::Pathfinding;
    m_currentWaypointIndex = 0;
    m_reachedDestination = false;

    requestPath();
    return true;
}

bool NavigationAgent::moveToActor(Actor *targetActor) {
    PROFILE_FUNCTION();

    if(!targetActor) {
        return false;
    }

    Transform *transform = targetActor->transform();
    if(!transform) {
        return false;
    }

    return moveTo(transform->position());
}

void NavigationAgent::stop() {
    m_state = NavigationState::Idle;
    m_hasTarget = false;
    m_reachedDestination = false;
    m_path.clear();
    m_currentWaypointIndex = 0;
    m_velocity = Vector3(0.0f);
    m_stuckTimer = 0.0f;
}

void NavigationAgent::pause(bool paused) {
    m_paused = paused;
}

void NavigationAgent::requestPath() {
    PROFILE_FUNCTION();

    Vector3 position = transform()->position();

    NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
    m_path = navSystem->findPath(scene(), position, m_target, m_agentType);
    if(!m_path.empty()) {
        m_currentWaypointIndex = 0;
        m_state = NavigationState::Moving;
        m_stuckTimer = 0.0f;
        m_lastPosition = position;

        pathFound();
    } else {
        pathFailed();
    }
}

void NavigationAgent::update() {
    PROFILE_FUNCTION();

    if(m_paused || m_state == NavigationState::Idle) {
        return;
    }

    if(m_state == NavigationState::Moving) {
        float deltaTime = Timer::deltaTime();

        updateMovement(deltaTime);
        checkWaypointReached();
        checkDestinationReached();
        handleStuckDetection(deltaTime);
        updateRotation(deltaTime);

        if(m_autoRepath && m_hasTarget && m_state == NavigationState::Moving) {
            checkPathDeviation();
        }
    }
}

void NavigationAgent::updateMovement(float deltaTime) {
    if(m_path.empty() || m_currentWaypointIndex >= m_path.size()) {
        if(m_autoBraking && m_velocity.length() > 0.01f) {
            float deceleration = m_maxAcceleration * 2.0f * deltaTime;
            float speed = m_velocity.length();
            if(speed > deceleration) {
                Vector3 vel = m_velocity;
                vel.normalize();
                m_velocity = m_velocity - vel * deceleration;
            } else {
                m_velocity = Vector3(0.0f);
            }
            transform()->setPosition(transform()->position() + m_velocity * deltaTime);
        }
        return;
    }

    Vector3 currentPos = transform()->position();
    Vector3 targetWaypoint = m_path[m_currentWaypointIndex];
    Vector3 direction = targetWaypoint - currentPos;
    float distance = direction.length();

    if(distance < m_stoppingDistance) {
        m_currentWaypointIndex++;
        if(m_currentWaypointIndex < m_path.size()) {
            waypointReached();
        }
        return;
    }

    Vector3 dir = direction;
    dir.normalize();
    Vector3 targetVelocity = dir * m_maxSpeed;

    if(m_autoBraking && m_currentWaypointIndex == m_path.size() - 1) {
        float speedFactor = CLAMP(distance / (m_stoppingDistance * 3.0f), 0.0f, 1.0f);
        targetVelocity = targetVelocity * speedFactor;
    }

    Vector3 deltaVel = targetVelocity - m_velocity;
    float acceleration = m_maxAcceleration * deltaTime;
    float deltaVelLen = deltaVel.length();
    if(deltaVelLen > acceleration) {
        deltaVel = deltaVel / deltaVelLen * acceleration;
    }
    m_velocity = m_velocity + deltaVel;

    float speed = m_velocity.length();
    if(speed > m_maxSpeed) {
        m_velocity = m_velocity / speed * m_maxSpeed;
    }

    transform()->setPosition(currentPos + m_velocity * deltaTime);
}

void NavigationAgent::updateRotation(float deltaTime) {
    PROFILE_FUNCTION();

    Vector3 direction(m_velocity);
    float speed = direction.normalize();
    if(speed < 0.01f) {
        return;
    }

    float currentYaw = transform()->rotation().y;
    currentYaw = fmod(currentYaw, 360.0f);      // 450° -> 90°
    if(currentYaw > 180.0f) currentYaw -= 360.0f; // 190° -> -170°
    if(currentYaw < -180.0f) currentYaw += 360.0f; // -190° -> 170°

    float targetYaw = atan2(direction.x, direction.z) * RAD2DEG;

    float maxDelta = m_angularSpeed * deltaTime;
    float deltaYaw = targetYaw - currentYaw;

    while(deltaYaw > 180.0f) deltaYaw -= 360.0f;
    while(deltaYaw < -180.0f) deltaYaw += 360.0f;

    if(fabs(deltaYaw) > maxDelta) {
        deltaYaw = (deltaYaw > 0.0f) ? maxDelta : -maxDelta;
    }

    float newYaw = currentYaw + deltaYaw;

    transform()->setRotation(Vector3(0.0f, newYaw, 0.0f));
}

void NavigationAgent::checkWaypointReached() {
    PROFILE_FUNCTION();

    if(m_path.empty() || m_currentWaypointIndex >= m_path.size()) {
        return;
    }

    Vector3 targetWaypoint = m_path[m_currentWaypointIndex];
    float distance = (targetWaypoint - transform()->position()).length();
    if(distance < m_stoppingDistance) {
        m_currentWaypointIndex++;
        if(m_currentWaypointIndex < m_path.size()) {
            waypointReached();
        }
    }
}

void NavigationAgent::checkDestinationReached() {
    PROFILE_FUNCTION();

    if(!m_hasTarget || m_path.empty() || m_currentWaypointIndex < m_path.size()) {
        return;
    }

    float distance = (m_target - transform()->position()).length();
    if(distance < m_stoppingDistance && m_state == NavigationState::Moving) {
        m_state = NavigationState::Idle;
        m_velocity = Vector3(0, 0, 0);
        m_reachedDestination = true;
        destinationReached();
    }
}

void NavigationAgent::handleStuckDetection(float deltaTime) {
    PROFILE_FUNCTION();

    if(m_state != NavigationState::Moving) {
        return;
    }

    Vector3 currentPos = transform()->position();
    float movement = (currentPos - m_lastPosition).length();
    m_lastPosition = currentPos;

    if(movement < 0.01f) {
        m_stuckTimer += deltaTime;
        if(m_stuckTimer > m_stuckThreshold) {
            stuck();
            if(m_hasTarget && m_autoRepath) {
                requestPath();
            } else {
                m_state = NavigationState::Idle;
            }
        }
    } else {
        m_stuckTimer = 0.0f;
        if(m_state == NavigationState::Stuck) {
            unstuck();
        }
    }
}

void NavigationAgent::checkPathDeviation() {
    if(m_path.empty() || m_currentWaypointIndex >= m_path.size()) {
        return;
    }

    Vector3 currentPos = transform()->position();
    Vector3 targetWaypoint = m_path[m_currentWaypointIndex];

    if(isOffPath(currentPos, m_pathDeviationThreshold)) {
        aDebug() << "Agent deviated from path, recalculating...";
        requestPath();
    }
}

bool NavigationAgent::isOffPath(const Vector3 &currentPos, float maxDeviation) const {
    if(m_path.size() < 2) {
        return false;
    }

    float minDist = FLT_MAX;
    for(size_t i = 0; i < m_path.size() - 1; ++i) {
        Vector3 a = m_path[i];
        Vector3 b = m_path[i + 1];

        float dist = distanceToSegment(currentPos, a, b);
        if(dist < minDist) {
            minDist = dist;
        }
    }

    return minDist > maxDeviation;
}

float NavigationAgent::distanceToSegment(const Vector3 &point, const Vector3 &a, const Vector3 &b) const {
    Vector3 ab = b - a;
    Vector3 ap = point - a;
    float t = ap.dot(ab) / ab.dot(ab);

    if(t < 0.0f) {
        return (point - a).length();
    }
    if(t > 1.0f) {
        return (point - b).length();
    }

    Vector3 projection = a + ab * t;
    return (point - projection).length();
}

void NavigationAgent::pathFound() {
    emitSignal(_SIGNAL(pathFound()));
}

void NavigationAgent::pathFailed() {
    m_state = NavigationState::Idle;
    m_hasTarget = false;
    emitSignal(_SIGNAL(pathFailed()));
}

void NavigationAgent::waypointReached() {
    emitSignal(_SIGNAL(waypointReached()));
}

void NavigationAgent::destinationReached() {
    emitSignal(_SIGNAL(destinationReached()));
}

void NavigationAgent::stuck() {
    m_state = NavigationState::Stuck;
    emitSignal(_SIGNAL(stuck()));
}

void NavigationAgent::unstuck() {
    m_state = NavigationState::Moving;
    emitSignal(_SIGNAL(unstuck()));
}

void NavigationAgent::drawGizmosSelected() {
    NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
    AgentType type = navSystem->agentType(m_agentType);

    Transform *t = transform();
    Gizmos::drawWireCylinder(Vector3(0.0f, type.height * 0.5f, 0.0f), type.radius, type.height, Vector4(0.5f, 1.0f, 0.5f, 1.0f), &t->worldTransform());

    if(!m_path.empty()) {
        Vector3Vector points;
        IndexVector indices;

        for(size_t i = 0; i < m_path.size(); ++i) {
            points.push_back(m_path[i]);
        }

        for(size_t i = 0; i < m_path.size() - 1; ++i) {
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        Gizmos::drawLines(points, indices, Vector4(0.0f, 1.0f, 1.0f, 1.0f));

        Gizmos::drawSolidSphere(m_target, 0.3f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    }
}
