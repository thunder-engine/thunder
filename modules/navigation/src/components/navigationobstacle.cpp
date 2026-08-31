#include "components/navigationobstacle.h"

#include "navigationsystem.h"

#include <transform.h>

NavigationObstacle::NavigationObstacle() :
        NativeBehaviour() {
    PROFILE_FUNCTION();
}

NavigationObstacle::~NavigationObstacle() {
    PROFILE_FUNCTION();

    unregisterObstacle();
}

void NavigationObstacle::update() {
    PROFILE_FUNCTION();

    Vector3 currentPosition = transform()->position();
    if(currentPosition != m_lastPosition) {
        m_lastPosition = currentPosition;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavigationObstacle::setRadius(float radius) {
    if(radius != m_radius) {
        m_radius = radius;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavigationObstacle::setHeight(float height) {
    if(height != m_height) {
        m_height = height;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavigationObstacle::registerObstacle() {
    if(m_obstacleId == 0) {
        m_lastPosition = transform()->position();

        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        if(!navSystem->addObstacle(*this)) {
            m_obstacleId = 0;
        }
    }
}

void NavigationObstacle::unregisterObstacle() {
    if(m_obstacleId != 0) {
        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        if(navSystem->removeObstacle(m_obstacleId)) {
            m_obstacleId = 0;
        }
    }
}

void NavigationObstacle::setObstacleRef(uint32_t ref) {
    m_obstacleId = ref;
}
