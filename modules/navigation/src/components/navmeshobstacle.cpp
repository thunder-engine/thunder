#include "components/navmeshobstacle.h"

#include "navigationsystem.h"

#include <transform.h>

/*!
    \class NavMeshObstacle
    \brief The NavMeshObstacle class adds a dynamic obstacle to a navigation mesh.
    \inmodule Navigation

    A navigation obstacle prevents agents from using the space occupied by its
    shape. The obstacle can be represented by a cylinder or a box and is
    automatically re-registered when its transform or geometry changes.

    \sa NavMeshAgent, NavigationSystem
*/

/*! \enum NavMeshObstacle::Shape
    Defines the geometric shape used by the navigation obstacle.

    \value Cylinder A cylindrical obstacle described by radius() and height().
    \value Box A box obstacle described by size().
*/

/*!
    Constructs a navigation obstacle with a cylindrical shape.
*/
NavMeshObstacle::NavMeshObstacle() :
        NativeBehaviour(),
        m_radius(0.6f),
        m_height(2.0f),
        m_shape(Cylinder),
        m_size(1.0f, 2.0f, 1.0f),
        m_obstacleId(0) {
    PROFILE_FUNCTION();
}

/*!
    Destroys the navigation obstacle and unregisters it from NavigationSystem.
*/
NavMeshObstacle::~NavMeshObstacle() {
    PROFILE_FUNCTION();

    unregisterObstacle();
}

/*!
    Updates the registered obstacle when its world position changes.
*/
void NavMeshObstacle::update() {
    PROFILE_FUNCTION();

    Vector3 currentPosition = transform()->position();
    if(currentPosition != m_lastPosition) {
        m_lastPosition = currentPosition;

        unregisterObstacle();
        registerObstacle();
    }
}

/*!
    Enables or disables the obstacle.

    An \a enabled value registers the obstacle in NavigationSystem, while a
    disabled value removes it from the navigation system while retaining its
    configuration.
*/
void NavMeshObstacle::setEnabled(bool enabled) {
    NativeBehaviour::setEnabled(enabled);

    if(enabled) {
        registerObstacle();
    } else {
        unregisterObstacle();
    }
}

/*!
    Returns the geometric shape of the obstacle.
*/
int NavMeshObstacle::shape() const {
    return m_shape;
}

/*!
    Sets the geometric shape of the obstacle.

    The \a shape value selects the geometry used by the obstacle. Invalid values
    are ignored, and a changed value re-registers the obstacle in
    NavigationSystem.
*/
void NavMeshObstacle::setShape(int shape) {
    if(shape < Cylinder || shape > Box || m_shape == shape) {
        return;
    }

    m_shape = shape;
    unregisterObstacle();
    registerObstacle();
}

/*!
    Returns the radius of a cylindrical obstacle.
*/
float NavMeshObstacle::radius() const {
    return m_radius;
}

/*!
    Sets the radius of a cylindrical obstacle.

    The \a radius value sets the new size in world units and re-registers the
    obstacle in NavigationSystem.
*/
void NavMeshObstacle::setRadius(float radius) {
    if(radius != m_radius) {
        m_radius = radius;

        unregisterObstacle();
        registerObstacle();
    }
}

/*!
    Returns the height of the obstacle.
*/
float NavMeshObstacle::height() const {
    return m_height;
}

/*!
    Sets the height of the obstacle.

    The \a height value sets the new size in world units and re-registers the
    obstacle in NavigationSystem.
*/
void NavMeshObstacle::setHeight(float height) {
    if(height != m_height) {
        m_height = height;

        unregisterObstacle();
        registerObstacle();
    }
}

/*!
    Returns the size of a box-shaped obstacle.
*/
Vector3 NavMeshObstacle::size() const {
    return m_size;
}

/*!
    Sets the size of a box-shaped obstacle.

    The \a size value sets the box dimensions in world units and re-registers the
    obstacle in NavigationSystem.
*/
void NavMeshObstacle::setSize(const Vector3 &size) {
    if(size != m_size) {
        m_size = size;

        unregisterObstacle();
        registerObstacle();
    }
}

/*!
    Registers the obstacle in NavigationSystem when it is not registered yet.
*/
void NavMeshObstacle::registerObstacle() {
    if(m_obstacleId == 0) {
        m_lastPosition = transform()->position();

        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        m_obstacleId = navSystem->addObstacle(*this);
    }
}

/*!
    Removes the obstacle from NavigationSystem when it is registered.
*/
void NavMeshObstacle::unregisterObstacle() {
    if(m_obstacleId != 0) {
        NavigationSystem *navSystem = static_cast<NavigationSystem *>(system());
        if(navSystem->removeObstacle(m_obstacleId)) {
            m_obstacleId = 0;
        }
    }
}
