#ifndef NAVIGATIONOBSTACLE_H
#define NAVIGATIONOBSTACLE_H

#include <nativebehaviour.h>
#include <navigation.h>

class NAVIGATION_EXPORT NavigationObstacle : public NativeBehaviour {
    A_OBJECT(NavigationObstacle, NativeBehaviour, Components/Navigation)

    A_PROPERTIES(
        A_PROPERTY(float, radius, NavigationObstacle::radius, NavigationObstacle::setRadius),
        A_PROPERTY(float, height, NavigationObstacle::height, NavigationObstacle::setHeight)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    NavigationObstacle();
    ~NavigationObstacle();

    void update() override;

    float radius() const { return m_radius; }
    void setRadius(float radius);

    float height() const { return m_height; }
    void setHeight(float height);

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

#endif // NAVIGATIONOBSTACLE_H
