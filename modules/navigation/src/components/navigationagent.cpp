#include "components/navigationagent.h"

#include "navigationsystem.h"

#include <transform.h>
#include <timer.h>
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

bool NavigationAgent::moveTo(const Vector3 &target) {
    PROFILE_FUNCTION();

    m_target = target;
    m_hasTarget = true;
    m_state = NavigationState::Pathfinding;
    m_currentWaypointIndex = 0;

    requestPath();
    return true;
}

bool NavigationAgent::moveTo(Actor *targetActor) {
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
    m_path.clear();
    m_currentWaypointIndex = 0;
    m_velocity = Vector3(0, 0, 0);
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
    }
}

void NavigationAgent::updateMovement(float deltaTime) {
    PROFILE_FUNCTION();

    if(m_path.empty() || m_currentWaypointIndex >= m_path.size()) {
        return;
    }

    Vector3 currentPos = transform()->position();
    Vector3 targetWaypoint = m_path[m_currentWaypointIndex];
    Vector3 direction = targetWaypoint - currentPos;
    float distance = direction.normalize();

    if(distance < m_stoppingDistance) {
        m_currentWaypointIndex++;
        if(m_currentWaypointIndex < m_path.size()) {
            waypointReached();
        }
        return;
    }

    Vector3 targetVelocity = direction * m_maxSpeed;
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
    direction.normalize();
    Quaternion targetRotation = Quaternion::lookRotation(direction, Vector3(0.0f, 1.0f, 0.0f));
    Quaternion currentRotation = transform()->rotation();
    Quaternion newRotation;
    newRotation.mix(currentRotation, targetRotation, deltaTime * 5.0f);
    transform()->setQuaternion(newRotation);
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
            if(m_hasTarget) {
                requestPath();
            }
        }
    } else {
        m_stuckTimer = 0.0f;
        if(m_state == NavigationState::Stuck) {
            unstuck();
        }
    }
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
