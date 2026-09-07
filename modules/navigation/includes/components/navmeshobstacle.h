#ifndef NAVMESHONOBSTACLE_H
#define NAVMESHONOBSTACLE_H

#include <nativebehaviour.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavMeshObstacle : public NativeBehaviour {
    A_OBJECT(NavMeshObstacle, NativeBehaviour, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(float, radius, NavMeshObstacle::radius, NavMeshObstacle::setRadius),
        A_PROPERTY(float, height, NavMeshObstacle::height, NavMeshObstacle::setHeight)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    NavMeshObstacle();
    ~NavMeshObstacle();

    void update() override;

    float radius() const { return m_radius; }
    void setRadius(float radius);

    float height() const { return m_height; }
    void setHeight(float height);

    void setEnabled(bool enabled) override;

    void setObstacleRef(uint32_t ref);

protected:
    void registerObstacle();
    void unregisterObstacle();

protected:
    Vector3 m_lastPosition;

    float m_radius = 0.6f;
    float m_height = 2.0f;

    uint32_t m_obstacleId = 0;
};

#endif // NAVMESHONOBSTACLE_H
