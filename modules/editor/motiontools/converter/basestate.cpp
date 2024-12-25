#include "basestate.h"

#include <resources/animationclip.h>

namespace {
    const char *gName("Name");
    const char *gClip("Clip");
    const char *gLoop("Loop");
}

BaseState::BaseState() :
        m_path(Template("", MetaType::name<AnimationClip>())),
        m_loop(false) {
}

QString BaseState::name() const {
    return objectName();
}

void BaseState::setName(const QString &name) {
    setObjectName(name);
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

Vector2 BaseState::defaultSize() const {
    return Vector2(170.0f, 40.0f);
}

Vector4 BaseState::color() const {
    return Vector4(0.141f, 0.384f, 0.514f, 1.0f);
}

void BaseState::saveUserData(QVariantMap &data) {
    data[gName] = objectName();

    data[gClip] = clip().path;
    data[gLoop] = loop();
}

void BaseState::loadUserData(const QVariantMap &data) {
    setObjectName(data[gName].toString());

    Template tpl;
    tpl.path = data[gClip].toString();
    tpl.type = clip().type;
    setClip(tpl);
    setLoop(data[gLoop].toBool());
}
