#ifndef NAVIGATIONAGENT_H
#define NAVIGATIONAGENT_H

#include <nativebehaviour.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavigationAgent : public NativeBehaviour {
    A_OBJECT(NavigationAgent, NativeBehaviour, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(int, agentType, NavigationAgent::agentType, NavigationAgent::setAgentType),
        A_PROPERTY(float, speed, NavigationAgent::speed, NavigationAgent::setSpeed),
        A_PROPERTY(float, angularSpeed, NavigationAgent::angularSpeed, NavigationAgent::setAngularSpeed),
        A_PROPERTY(float, acceleration, NavigationAgent::acceleration, NavigationAgent::setAcceleration),
        A_PROPERTY(float, stoppingDistance, NavigationAgent::stoppingDistance, NavigationAgent::setStoppingDistance),
        A_PROPERTY(bool, autoBraking, NavigationAgent::autoBraking, NavigationAgent::setAutoBraking),
        A_PROPERTY(bool, autoRepath, NavigationAgent::autoRepath, NavigationAgent::setAutoRepath)
        )
    A_METHODS(
        A_SIGNAL(NavigationAgent::pathFound),
        A_SIGNAL(NavigationAgent::pathFailed),
        A_SIGNAL(NavigationAgent::waypointReached),
        A_SIGNAL(NavigationAgent::destinationReached),
        A_SIGNAL(NavigationAgent::stuck),
        A_SIGNAL(NavigationAgent::unstuck),
        A_METHOD(bool, NavigationAgent::moveTo),
        A_METHOD(bool, NavigationAgent::moveToActor),
        A_METHOD(void, NavigationAgent::stop),
        A_METHOD(void, NavigationAgent::pause)
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
    NavigationAgent();
    ~NavigationAgent();

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

#endif // NAVIGATIONAGENT_H
