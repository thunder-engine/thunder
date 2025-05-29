#include "components/checkbox.h"

#include "components/frame.h"
#include "components/image.h"
#include "components/recttransform.h"
#include "components/label.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <timer.h>

namespace  {
    const char *gKnob("knob");
}

/*!
    \class CheckBox
    \brief A CheckBox is an option button that can be switched on or off.
    \inmodule Gui

    The CheckBox class represents an option button that can be toggled between two states: "on" or "off."
    It is commonly used in graphical user interfaces (GUIs) to allow users to select or deselect specific options or features, often in forms or settings.
*/

CheckBox::CheckBox() :
        AbstractButton(),
        m_knobColor(1.0f) {

    setCheckable(true);
}
/*!
    Returns the graphical knob component.
*/
Image *CheckBox::knobGraphic() const {
    return static_cast<Image *>(subWidget(gKnob));
}
/*!
    Sets the graphical \a knob component.
*/
void CheckBox::setKnobGraphic(Image *knob) {
    setSubWidget(gKnob, knob);

    if(knob) {
        knob->setColor(m_knobColor);
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

    Image *knob = knobGraphic();
    if(knob) {
        knob->setColor(m_knobColor);
    }
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
    }

    Frame *frame = background();
    if(frame) {
        RectTransform *rect = frame->rectTransform();
        rect->setAnchors(Vector2(flag ? 1.0f : 0.0f, 0.5f), Vector2(flag ? 1.0f : 0.0f, 0.5f));
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
