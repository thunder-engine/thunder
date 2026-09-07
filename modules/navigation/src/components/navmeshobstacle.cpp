#include "components/navmeshobstacle.h"

#include "navigationsystem.h"

#include <transform.h>

NavMeshObstacle::NavMeshObstacle() :
        NativeBehaviour() {
    PROFILE_FUNCTION();
}

NavMeshObstacle::~NavMeshObstacle() {
    PROFILE_FUNCTION();

    unregisterObstacle();
}

void NavMeshObstacle::update() {
    PROFILE_FUNCTION();

    Vector3 currentPosition = transform()->position();
    if(currentPosition != m_lastPosition) {
        m_lastPosition = currentPosition;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavMeshObstacle::setEnabled(bool enabled) {
    NativeBehaviour::setEnabled(enabled);

    if(enabled) {
        registerObstacle();
    } else {
        unregisterObstacle();
    }
}

void NavMeshObstacle::setRadius(float radius) {
    if(radius != m_radius) {
        m_radius = radius;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavMeshObstacle::setHeight(float height) {
    if(height != m_height) {
        m_height = height;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavMeshObstacle::registerObstacle() {
    if(m_obstacleId == 0) {
        m_lastPosition = transform()->position();

        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        if(!navSystem->addObstacle(*this)) {
            m_obstacleId = 0;
        }
    }
}

void NavMeshObstacle::unregisterObstacle() {
    if(m_obstacleId != 0) {
        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        if(navSystem->removeObstacle(m_obstacleId)) {
            m_obstacleId = 0;
        }
    }
}

void NavMeshObstacle::setObstacleRef(uint32_t ref) {
    m_obstacleId = ref;
}
