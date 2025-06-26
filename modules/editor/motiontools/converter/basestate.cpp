#include "basestate.h"

#include <resources/animationclip.h>

BaseState::BaseState() :
        m_clip(nullptr),
        m_loop(false) {
}

AnimationClip *BaseState::clip() const {
    return m_clip;
}

void BaseState::setClip(AnimationClip *clip) {
    m_clip = clip;
}

bool BaseState::loop() const {
    return m_loop;
}

void BaseState::setLoop(bool loop) {
    m_loop = loop;
}

Vector2 BaseState::defaultSize() const {
    return Vector2(170.0f, 40.0f);
}

Vector4 BaseState::color() const {
    return Vector4(0.141f, 0.384f, 0.514f, 1.0f);
}
