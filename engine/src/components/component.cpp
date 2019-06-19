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

Component::Component() :
        p_ptr(new ComponentPrivate) {

}

Component::~Component() {
    delete p_ptr;
}

Actor *Component::actor() const {
    return (static_cast<Actor *>(parent()));
}

bool Component::isEnable() const {
    return p_ptr->m_Enable;
}

void Component::setEnable(bool enable) {
    p_ptr->m_Enable = enable;
}

bool Component::isStarted() const {
    return p_ptr->m_Started;
}

void Component::setStarted(bool started) {
    p_ptr->m_Started = started;
}

bool Component::isSerializable() const {
    return (!actor()->isPrefab() && actor()->isSerializable());
}

#ifdef NEXT_SHARED
bool Component::drawHandles() { return false; }
#endif
