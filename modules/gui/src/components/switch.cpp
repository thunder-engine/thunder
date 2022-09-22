#include "components/switch.h"

#include "components/frame.h"
#include "components/recttransform.h"
#include "components/label.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/sprite.h>

#include <timer.h>

namespace  {
    const char *gKnob = "Knob";
}

Switch::Switch() :
    AbstractButton(),
    m_knobColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
    m_knobGraphic(nullptr),
    m_switchDuration(0.2f),
    m_currentFade(1.0f) {

    setCheckable(true);
}

Switch::~Switch() {

}

float Switch::switchDuration() const {
    return m_switchDuration;
}
void Switch::setSwitchDuration(float duration) {
    m_switchDuration = duration;
}

Frame *Switch::knobGraphic() const {
    return m_knobGraphic;
}

void Switch::setKnobGraphic(Frame *panel) {
    if(m_knobGraphic != panel) {
        disconnect(m_knobGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_knobGraphic = panel;
        if(m_knobGraphic) {
            connect(m_knobGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_knobGraphic->setColor(m_knobColor);
        }
    }
}

Vector4 Switch::knobColor() const {
    return m_knobColor;
}
void Switch::setKnobColor(const Vector4 color) {
    m_knobColor = color;
    if(m_knobGraphic) {
        m_knobGraphic->setColor(m_knobColor);
    }
}

void Switch::update() {
    AbstractButton::update();

    if(m_currentFade < 1.0f && m_knobGraphic) {
        m_currentFade += 1.0f / m_switchDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

        Actor *actor = m_knobGraphic->actor();
        RectTransform *transform = dynamic_cast<RectTransform *>(actor->transform());
        if(transform) {
            if(m_checked) {
                transform->setPivot(Vector2(MIX(1.0f, 0.0f, m_currentFade), 0.5f));
            } else {
                transform->setPivot(Vector2(MIX(0.0f, 1.0f, m_currentFade), 0.5f));
            }
        }
    }
}

void Switch::checkStateSet() {
    AbstractButton::checkStateSet();
    m_currentFade = 0.0f;
}
/*!
    \internal
*/
void Switch::loadUserData(const VariantMap &data) {
    AbstractButton::loadUserData(data);
    {
        auto it = data.find(gKnob);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setKnobGraphic(dynamic_cast<Frame *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap Switch::saveUserData() const {
    VariantMap result = AbstractButton::saveUserData();
    {
        if(m_knobGraphic) {
            result[gKnob] = int(m_knobGraphic->uuid());
        }
    }
    return result;
}

void Switch::setMirrored(bool flag) {
    Label *lbl = label();
    if(lbl) {
        RectTransform *rect = dynamic_cast<RectTransform *>(lbl->actor()->transform());
        if(rect) {
            rect->setOffsetMin(Vector2(flag ? 5.0f : 43.0f, 1.0f));
            rect->setOffsetMax(Vector2(flag ? 43.0f : 5.0f, 2.0f));
        }

        if(flag) {
            lbl->setAlign(Alignment::Middle | Alignment::Right);
        } else {
            lbl->setAlign(Alignment::Middle | Alignment::Left);
        }
    }

    Frame *frame = background();
    if(frame) {
        RectTransform *rect = frame->rectTransform();
        rect->setAnchors(Vector2(flag ? 1.0f : 0.0f, 0.0f), Vector2(flag ? 1.0f : 0.0f, 1.0f));
        rect->setPivot(Vector2(flag ? 1.0f : 0.0f, 0.5f));
    }

    AbstractButton::setMirrored(flag);
}
/*!
    \internal
*/
void Switch::composeComponent() {
    AbstractButton::composeComponent();

    RectTransform *rect = dynamic_cast<RectTransform *>(actor()->transform());
    if(rect) {
        rect->setSize(Vector2(100.0f, 30.0f));

        RectTransform *labelRect = static_cast<RectTransform *>(label()->actor()->transform());
        labelRect->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f));
        labelRect->setOffsetMin(Vector2(43.0f, 1.0f));
        labelRect->setOffsetMax(Vector2(5.0f, 2.0f));

        label()->setAlign(label()->align() | Alignment::Left);

        RectTransform *backRect = static_cast<RectTransform *>(background()->actor()->transform());
        backRect->setAnchors(Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f));
        backRect->setPivot(Vector2(0.0f, 0.5f));
        backRect->setSize(Vector2(40.0f, 20.0f));

        // Add knob
        Actor *knob = Engine::composeActor("Frame", "Knob", background()->actor());
        Frame *frame = static_cast<Frame *>(knob->component("Frame"));
        frame->setCorners(background()->corners());
        setKnobGraphic(frame);

        RectTransform *knobRect = frame->rectTransform();
        Vector2 size = backRect->size();
        size.x *= 0.5f;
        knobRect->setAnchors(Vector2(0.5f, 0.0f), Vector2(0.5f, 1.0f));
        knobRect->setPivot(Vector2(1.0f, 0.5f));
        knobRect->setSize(size);
    }
}
/*!
    \internal
*/
void Switch::onReferenceDestroyed() {
    AbstractButton::onReferenceDestroyed();

    Object *object = sender();
    if(m_knobGraphic == object) {
        m_knobGraphic = nullptr;
    }
}
