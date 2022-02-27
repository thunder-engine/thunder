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
    p_ptr = nullptr;
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
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
bool Component::isStarted() const {
    return p_ptr->m_Started;
}
/*!
    Marks component as \a started.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Component::setStarted(bool started) {
    p_ptr->m_Started = started;
}
/*!
    Returns a translated version of \a source text; otherwise returns source text if no appropriate translated string is available.
*/
string Component::tr(const string &source) {
    return Engine::translate(source);
}
/*!
    This method will be triggered in case of Actor will change its own parent.
    \internal
*/
void Component::actorParentChanged() {

}
/*!
    Returns true in case of the component can be rendered on the screen; otherwise returns false.
*/
bool Component::isRenderable() const {
    return false;
}
/*!
    \internal
*/
bool Component::isPostProcessVolume() const {
    return false;
}
/*!
    \internal
*/
bool Component::isSerializable() const {
    return (clonedFrom() == 0);
}
/*!
    \internal
*/
bool Component::isComponent() const {
    return true;
}

void Component::onReferenceDestroyed() {

}

#ifdef SHARED_DEFINE
bool Component::drawHandles(ObjectList &selected) { A_UNUSED(selected); return false; }

bool Component::isSelected(ObjectList &selected) {
    for(auto it : selected) {
        if(it == parent()) {
            return true;
        }
    }
    return false;
}

#endif
