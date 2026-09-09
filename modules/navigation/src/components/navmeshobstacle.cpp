#include "components/navmeshobstacle.h"

#include "navigationsystem.h"

#include <transform.h>

NavMeshObstacle::NavMeshObstacle() :
        NativeBehaviour(),
        m_radius(0.6f),
        m_height(2.0f),
        m_shape(Cylinder),
        m_size(1.0f, 2.0f, 1.0f),
        m_obstacleId(0) {
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

int NavMeshObstacle::shape() const {
    return m_shape;
}

void NavMeshObstacle::setShape(int shape) {
    if(shape < Cylinder || shape > Box || m_shape == shape) {
        return;
    }

    m_shape = shape;
    unregisterObstacle();
    registerObstacle();
}

float NavMeshObstacle::radius() const {
    return m_radius;
}

void NavMeshObstacle::setRadius(float radius) {
    if(radius != m_radius) {
        m_radius = radius;

        unregisterObstacle();
        registerObstacle();
    }
}

float NavMeshObstacle::height() const {
    return m_height;
}

void NavMeshObstacle::setHeight(float height) {
    if(height != m_height) {
        m_height = height;

        unregisterObstacle();
        registerObstacle();
    }
}

Vector3 NavMeshObstacle::size() const {
    return m_size;
}

void NavMeshObstacle::setSize(const Vector3 &size) {
    if(size != m_size) {
        m_size = size;

        unregisterObstacle();
        registerObstacle();
    }
}

void NavMeshObstacle::registerObstacle() {
    if(m_obstacleId == 0) {
        m_lastPosition = transform()->position();

        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        m_obstacleId = navSystem->addObstacle(*this);
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
