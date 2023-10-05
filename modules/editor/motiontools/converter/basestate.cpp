#include "basestate.h"

#include <resources/animationclip.h>

BaseState::BaseState() {
    m_path = Template("", MetaType::type<AnimationClip *>());
    m_loop = false;
}

Template BaseState::clip() const {
    return m_path;
}

void BaseState::setClip(const Template &path) {
    m_path.path = path.path;
    emit updated();
}

bool BaseState::loop() const {
    return m_loop;
}

void BaseState::setLoop(bool loop) {
    m_loop = loop;
    emit updated();
}
