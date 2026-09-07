#ifndef NAVMESHAGENT_H
#define NAVMESHAGENT_H

#include <nativebehaviour.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavMeshAgent : public NativeBehaviour {
    A_OBJECT(NavMeshAgent, NativeBehaviour, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(int, agentType, NavMeshAgent::agentType, NavMeshAgent::setAgentType),
        A_PROPERTY(float, speed, NavMeshAgent::speed, NavMeshAgent::setSpeed),
        A_PROPERTY(float, angularSpeed, NavMeshAgent::angularSpeed, NavMeshAgent::setAngularSpeed),
        A_PROPERTY(float, acceleration, NavMeshAgent::acceleration, NavMeshAgent::setAcceleration),
        A_PROPERTY(float, stoppingDistance, NavMeshAgent::stoppingDistance, NavMeshAgent::setStoppingDistance),
        A_PROPERTY(bool, autoBraking, NavMeshAgent::autoBraking, NavMeshAgent::setAutoBraking),
        A_PROPERTY(bool, autoRepath, NavMeshAgent::autoRepath, NavMeshAgent::setAutoRepath)
        )
    A_METHODS(
        A_SIGNAL(NavMeshAgent::pathFound),
        A_SIGNAL(NavMeshAgent::pathFailed),
        A_SIGNAL(NavMeshAgent::waypointReached),
        A_SIGNAL(NavMeshAgent::destinationReached),
        A_SIGNAL(NavMeshAgent::stuck),
        A_SIGNAL(NavMeshAgent::unstuck),
        A_METHOD(bool, NavMeshAgent::moveTo),
        A_METHOD(bool, NavMeshAgent::moveToActor),
        A_METHOD(void, NavMeshAgent::stop),
        A_METHOD(void, NavMeshAgent::pause)
        )
    A_NOENUMS()

public:
    enum NavigationState {
        Idle,
        Moving,
        Pathfinding,
        Stuck,
        Arrived
    };

public:
    NavMeshAgent();
    ~NavMeshAgent();

    int agentType() const;
    void setAgentType(int type);

    float speed() const;
    void setSpeed(float speed);

    float angularSpeed() const;
    void setAngularSpeed(float angularSpeed);

    float acceleration() const;
    void setAcceleration(float acceleration);

    float stoppingDistance() const;
    void setStoppingDistance(float stoppingDistance);

    bool autoBraking() const;
    void setAutoBraking(bool autoBraking);

    bool autoRepath() const;
    void setAutoRepath(bool autoRepath);

    bool moveTo(const Vector3 &target);
    bool moveToActor(Actor *actor);
    void stop();
    void pause(bool paused);

    Vector3 velocity() const { return m_velocity; }
    NavigationState state() const { return m_state; }
    const Vector3 &target() const { return m_target; }
    const Vector3Vector &path() const { return m_path; }

public: // signals
    void pathFound();
    void pathFailed();
    void waypointReached();
    void destinationReached();
    void stuck();
    void unstuck();

protected:
    void update() override;

    void checkWaypointReached();
    void checkDestinationReached();
    void updateMovement(float deltaTime);
    void handleStuckDetection(float deltaTime);
    void updateRotation(float deltaTime);
    void requestPath();

    void checkPathDeviation();
    bool isOffPath(const Vector3 &currentPos, float maxDeviation) const;
    float distanceToSegment(const Vector3 &point, const Vector3 &a, const Vector3 &b) const;

    void drawGizmosSelected() override;

protected:
    int m_agentType = 0;

    float m_maxSpeed = 3.0f;
    float m_angularSpeed = 120.0f;
    float m_maxAcceleration = 8.0f;
    float m_stoppingDistance = 0.5f;
    bool m_autoBraking = true;
    bool m_autoRepath = true;

    Vector3Vector m_path;
    size_t m_currentWaypointIndex = 0;

    NavigationState m_state = NavigationState::Idle;
    Vector3 m_target;
    Vector3 m_velocity;
    Vector3 m_lastPosition;
    float m_stuckTimer = 0.0f;
    float m_stuckThreshold = 2.0f;

    bool m_paused = false;
    bool m_hasTarget = false;
    bool m_reachedDestination = false;

    float m_pathDeviationThreshold = 1.0f;
};

#endif // NAVMESHAGENT_H
