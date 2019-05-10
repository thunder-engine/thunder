#include "components/component.h"

#include "components/actor.h"

Component::Component() :
        Object(),
        m_Enable(true),
        m_Started(false) {

}

Actor *Component::actor() const {
    return (static_cast<Actor *>(parent()));
}

bool Component::isEnable() const {
    return m_Enable;
}

void Component::setEnable(bool enable) {
    m_Enable    = enable;
}

bool Component::isStarted() const {
    return m_Started;
}

void Component::setStarted(bool started) {
    m_Started   = started;
}

bool Component::isSerializable() const {
    return (!actor()->isPrefab() && actor()->isSerializable());
}
