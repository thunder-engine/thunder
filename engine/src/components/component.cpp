#include "components/component.h"

#include "components/actor.h"

class ComponentPrivate {
public:
    ComponentPrivate() :
        m_Enable(true),
        m_Started(false) {

    }

    bool m_Enable;

    bool m_Started;
};
/*!
    \class Component
    \brief Base class for everything attached to Actor.
    \inmodule Engine

    The Component class is a base class for each aspect of the actor, and how it interacts with the world.
    \note This class must be a superclass only and shouldn't be created manually.
*/
Component::Component() :
        p_ptr(new ComponentPrivate) {

}
Component::~Component() {
    delete p_ptr;
}
/*!
    Returns a pointer to the actor to which the component is attached.
*/
Actor *Component::actor() const {
    return (static_cast<Actor *>(parent()));
}
/*!
    Returns true if the component is enabled; otherwise returns false.
*/
bool Component::isEnabled() const {
    return p_ptr->m_Enable;
}
/*!
    Sets current state of component to \a enabled or disabled.
    \note The disabled component will be created but not affect the Actor. For example, MeshRender component will not draw a mesh.
*/
void Component::setEnabled(bool enabled) {
    p_ptr->m_Enable = enabled;
}
/*!
    Returns true if the component is flagged as started; otherwise returns false.
    \note This function is used for internal purposes and shouldn't be called manually.

    \internal
*/
bool Component::isStarted() const {
    return p_ptr->m_Started;
}
/*!
    Marks component as \a started.
    \note This function is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Component::setStarted(bool started) {
    p_ptr->m_Started = started;
}

bool Component::isSerializable() const {
    return ((!actor()->isPrefab() || clonedFrom() == 0) && static_cast<Object*>(actor())->isSerializable());
}

#ifdef NEXT_SHARED
bool Component::drawHandles() { return false; }
#endif
