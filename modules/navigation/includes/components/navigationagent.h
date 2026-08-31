#ifndef NAVIGATIONAGENT_H
#define NAVIGATIONAGENT_H

#include <nativebehaviour.h>

class NavigationAgent : public NativeBehaviour {
    A_OBJECT(NavigationAgent, NativeBehaviour, Components/Navigation)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(NavigationAgent::pathFound),
        A_SIGNAL(NavigationAgent::pathFailed),
        A_SIGNAL(NavigationAgent::waypointReached),
        A_SIGNAL(NavigationAgent::destinationReached),
        A_SIGNAL(NavigationAgent::stuck),
        A_SIGNAL(NavigationAgent::unstuck)
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

    bool moveTo(const Vector3 &target);
    bool moveTo(Actor *targetActor);
    void stop();
    void pause(bool paused);

    NavigationState state() const { return m_state; }
    const Vector3 &target() const { return m_target; }
    const std::vector<Vector3> &path() const { return m_path; }
    Vector3 velocity() const { return m_velocity; }

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

protected:
    int m_agentType = 0;

    float m_maxSpeed = 3.0f;
    float m_maxAcceleration = 8.0f;
    float m_stoppingDistance = 0.5f;

    std::vector<Vector3> m_path;
    size_t m_currentWaypointIndex = 0;

    NavigationState m_state = NavigationState::Idle;
    Vector3 m_target;
    Vector3 m_velocity;
    Vector3 m_lastPosition;
    float m_stuckTimer = 0.0f;
    float m_stuckThreshold = 2.0f;

    bool m_paused = false;
    bool m_hasTarget = false;

};

#endif // NAVIGATIONAGENT_H
