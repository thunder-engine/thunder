#include "components/checkbox.h"

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
    \class CheckBox
    \brief The Switch class is a UI component that acts as a switch or toggle button with a graphical knob.
    \inmodule Gui

    The Switch class provides a customizable switch button with an animated graphical knob.
    It inherits functionality from the AbstractButton class and extends it to handle knob-related features and animations.
*/

CheckBox::CheckBox() :
        AbstractButton(),
        m_knobColor(1.0f),
        m_knobGraphic(nullptr) {

    setCheckable(true);
}
/*!
    Returns the graphical knob component.
*/
Image *CheckBox::knobGraphic() const {
    return m_knobGraphic;
}
/*!
    Sets the graphical \a knob component.
*/
void CheckBox::setKnobGraphic(Image *knob) {
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
Vector4 CheckBox::knobColor() const {
    return m_knobColor;
}
/*!
    Sets the \a color of the graphical knob.
*/
void CheckBox::setKnobColor(const Vector4 color) {
    m_knobColor = color;
    if(m_knobGraphic) {
        m_knobGraphic->setColor(m_knobColor);
    }
}
/*!
    \internal
    Overrides the loadUserData method to handle loading knob data.
*/
void CheckBox::loadUserData(const VariantMap &data) {
    AbstractButton::loadUserData(data);

    auto it = data.find(gKnob);
    if(it != data.end()) {
        uint32_t uuid = uint32_t((*it).second.toInt());
        Object *object = Engine::findObject(uuid, Engine::findRoot(this));
        setKnobGraphic(dynamic_cast<Image *>(object));
    }
}
/*!
    \internal
    Overrides the saveUserData method to handle saving knob data.
*/
VariantMap CheckBox::saveUserData() const {
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
void CheckBox::setMirrored(bool flag) {
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
    Overrides the checkStateSet method to handle state changes.
*/
void CheckBox::checkStateSet() {
    AbstractButton::checkStateSet();

    knobGraphic()->actor()->setEnabled(m_checked);
}
/*!
    \internal
    Overrides the composeComponent method to create the switch component.
*/
void CheckBox::composeComponent() {
    AbstractButton::composeComponent();

    Frame *back = background();
    if(back) {
        RectTransform *backRect = back->rectTransform();
        backRect->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
        backRect->setPivot(Vector2(0.0f, 0.5f));
        backRect->setSize(Vector2(16.0f, 16.0f));

        Label *label = AbstractButton::label();
        if(label) {
            label->setAlign(Alignment::Middle | Alignment::Left);
            RectTransform *labelRect = label->rectTransform();
            labelRect->setMargin(Vector4(0.0f, 0.0f, 0.0f, backRect->size().x + back->corners().x));
        }

        // Add knob
        Actor *knob = Engine::composeActor("Image", "Knob", background()->actor());
        Image *image = knob->getComponent<Image>();
        if(image) {
            Sprite *arrow = Engine::loadResource<Sprite>(".embedded/ui.png");
            image->setSprite(arrow);
            image->setItem("Check");

            RectTransform *knobRect = image->rectTransform();
            Vector2 size = backRect->size();
            knobRect->setSize(Vector2(16, 8));

            knob->setEnabled(m_checked);
        }
        setKnobGraphic(image);
    }

    // Disable Icon by the default
    icon()->actor()->setEnabled(false);
}
/*!
    \internal
    Overrides the onReferenceDestroyed method to handle knob destruction.
*/
void CheckBox::onReferenceDestroyed() {
    AbstractButton::onReferenceDestroyed();

    Object *object = sender();
    if(m_knobGraphic == object) {
        m_knobGraphic = nullptr;
    }
}
