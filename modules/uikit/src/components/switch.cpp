#include "components/switch.h"

#include "components/frame.h"
#include "components/image.h"
#include "components/recttransform.h"
#include "components/label.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <timer.h>

namespace  {
    const char *gKnob = "Knob";
}

/*!
    \class Switch
    \brief The Switch class is a UI component that acts as a switch or toggle button with a graphical knob.
    \inmodule Gui

    The Switch class provides a customizable switch button with an animated graphical knob.
    It inherits functionality from the AbstractButton class and extends it to handle knob-related features and animations.
*/

Switch::Switch() :
        AbstractButton(),
        m_knobColor(1.0f),
        m_knobGraphic(nullptr),
        m_switchDuration(0.2f),
        m_currentFade(1.0f) {

    setCheckable(true);
}
/*!
    Returns the switch animation duration in seconds.
*/
float Switch::switchDuration() const {
    return m_switchDuration;
}
/*!
    Sets the switch animation \a duration in seconds.
*/
void Switch::setSwitchDuration(float duration) {
    m_switchDuration = duration;
}
/*!
    Returns the graphical knob component.
*/
Frame *Switch::knobGraphic() const {
    return m_knobGraphic;
}
/*!
    Sets the graphical \a knob component.
*/
void Switch::setKnobGraphic(Frame *knob) {
    if(m_knobGraphic != knob) {
        disconnect(m_knobGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_knobGraphic = knob;
        if(m_knobGraphic) {
            connect(m_knobGraphic, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_knobGraphic->setColor(m_knobColor);
        }
    }
}
/*!
    Returns the color of the graphical knob.
*/
Vector4 Switch::knobColor() const {
    return m_knobColor;
}
/*!
    Sets the \a color of the graphical knob.
*/
void Switch::setKnobColor(const Vector4 color) {
    m_knobColor = color;
    if(m_knobGraphic) {
        m_knobGraphic->setColor(m_knobColor);
    }
}
/*!
    \internal
    Overrides the update method to handle knob animation.
*/
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
/*!
    \internal
    Overrides the checkStateSet method to handle state changes.
*/
void Switch::checkStateSet() {
    AbstractButton::checkStateSet();
    m_currentFade = 0.0f;
}
/*!
    \internal
    Overrides the loadUserData method to handle loading knob data.
*/
void Switch::loadUserData(const VariantMap &data) {
    AbstractButton::loadUserData(data);

    auto it = data.find(gKnob);
    if(it != data.end()) {
        uint32_t uuid = uint32_t((*it).second.toInt());
        Object *object = Engine::findObject(uuid);
        setKnobGraphic(dynamic_cast<Frame *>(object));
    }
}
/*!
    \internal
    Overrides the saveUserData method to handle saving knob data.
*/
VariantMap Switch::saveUserData() const {
    VariantMap result = AbstractButton::saveUserData();

    if(m_knobGraphic) {
        result[gKnob] = int(m_knobGraphic->uuid());
    }

    return result;
}
/*!
    \internal
    Overrides the setMirrored method to handle mirrored UI elements.
*/
void Switch::setMirrored(bool flag) {
    Label *lbl = label();
    if(lbl) {
        RectTransform *rect = lbl->rectTransform();
        rect->setMargin(Vector4(1.0f, flag ? 43.0f : 5.0f, 2.0f, flag ? 5.0f : 43.0f));

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
    Overrides the composeComponent method to create the switch component.
*/
void Switch::composeComponent() {
    AbstractButton::composeComponent();

    Frame *back = background();
    if(back) {
        RectTransform *backRect = back->rectTransform();
        backRect->setAnchors(Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f));
        backRect->setPivot(Vector2(0.0f, 0.5f));
        backRect->setSize(Vector2(40.0f, 20.0f));

        Label *label = AbstractButton::label();
        if(label) {
            label->setAlign(Alignment::Middle | Alignment::Left);
            RectTransform *labelRect = label->rectTransform();
            labelRect->setMargin(Vector4(0.0f, 0.0f, 0.0f, backRect->size().x + back->corners().x));
        }

        // Add knob
        Actor *knob = Engine::composeActor("Frame", "Knob", background()->actor());
        Frame *frame = knob->getComponent<Frame>();
        frame->setCorners(background()->corners());
        setKnobGraphic(frame);

        RectTransform *knobRect = frame->rectTransform();
        Vector2 size = backRect->size();
        size.x *= 0.5f;
        knobRect->setAnchors(Vector2(0.5f, 0.0f), Vector2(0.5f, 1.0f));
        knobRect->setPivot(Vector2(1.0f, 0.5f));
        knobRect->setSize(size);
    }

    // Disable Icon by the default
    icon()->actor()->setEnabled(false);
}
/*!
    \internal
    Overrides the onReferenceDestroyed method to handle knob destruction.
*/
void Switch::onReferenceDestroyed() {
    AbstractButton::onReferenceDestroyed();

    Object *object = sender();
    if(m_knobGraphic == object) {
        m_knobGraphic = nullptr;
    }
}
