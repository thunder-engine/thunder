#include "basestate.h"

#include <resources/animationclip.h>

BaseState::BaseState() :
        m_path(Template("", MetaType::type<AnimationClip *>())),
        m_loop(false) {
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

Vector4 BaseState::color() const {
    return Vector4(0.141f, 0.384f, 0.514f, 1.0f);
}
