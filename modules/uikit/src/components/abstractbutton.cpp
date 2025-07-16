#include "components/abstractbutton.h"

#include "components/recttransform.h"
#include "components/frame.h"
#include "components/label.h"
#include "components/image.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/stylesheet.h>

#include <input.h>
#include <timer.h>
#include <log.h>

namespace  {
    const char *gBackground("background");
    const char *gLabel("label");
    const char *gIcon("icon");

    const char *gImageClass("Image");
    const char *gFrameClass("Frame");
    const char *gLabelClass("Label");

    const float gCorner = 4.0f;
}
/*!
    \class AbstractButton
    \brief The AbstractButton class represents a base class for interactive buttons in a graphical user interface.
    \inmodule Gui

    The AbstractButton class provides a foundation for creating interactive buttons within a graphical user interface.
    It allows customization of various visual properties and handles user interaction events.
    Internal methods are marked as internal and are intended for use within the framework rather than by external code.
*/

AbstractButton::AbstractButton() :
        Widget(),
        m_normalColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
        m_highlightedColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f)),
        m_pressedColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f)),
        m_textColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
        m_iconSize(16.0f),
        m_fadeDuration(0.2f),
        m_currentFade(1.0f),
        m_hovered(false),
        m_mirrored(false),
        m_checkable(false),
        m_checked(false),
        m_exclusive(false) {

}
/*!
    Returns the text displayed on the button.
*/
String AbstractButton::text() const {
    Label *lbl = label();
    if(lbl) {
        return lbl->text();
    }
    return String();
}
/*!
    Sets the \a text displayed on the button.
*/
void AbstractButton::setText(const String text) {
    Label *lbl = label();
    if(lbl) {
        lbl->setText(text);
    }
}
/*!
    Returns the background frame object associated with the button.
*/
Frame *AbstractButton::background() const {
    return static_cast<Frame *>(subWidget(gBackground));
}
/*!
    Sets the background \a frame of the button.
*/
void AbstractButton::setBackground(Frame *frame) {
    setSubWidget(gBackground, frame);

    if(frame) {
        frame->setColor(m_normalColor);
    }
}
/*!
    Returns the label object associated with the button.
*/
Label *AbstractButton::label() const {
    return static_cast<Label *>(subWidget(gLabel));
}
/*!
    Sets the \a label associated with the button.
*/
void AbstractButton::setLabel(Label *label) {
    setSubWidget(gLabel, label);
}
/*!
     Returns the icon associated with the button.
*/
Image *AbstractButton::icon() const {
    return static_cast<Image *>(subWidget(gIcon));
}
/*!
    Sets the icon \a image associated with the button.
*/
void AbstractButton::setIcon(Image *image) {
    setSubWidget(gIcon, image);

    if(image) {
        RectTransform *rect = image->rectTransform();
        if(rect) {
            rect->setAnchors(Vector2(0.5f), Vector2(0.5f));
            rect->setSize(m_iconSize);
        }
    }
}
/*!
     Returns the size of the icon.
*/
Vector2 AbstractButton::iconSize() const {
    return m_iconSize;
}
/*!
    Sets the \a size of the icon.
*/
void AbstractButton::setIconSize(Vector2 size) {
    m_iconSize = size;
    Image *img = icon();
    if(img) {
        img->rectTransform()->setSize(m_iconSize);
    }
}
/*!
    Returns the fade duration used for visual effects.
*/
float AbstractButton::fadeDuration() const {
    return m_fadeDuration;
}
/*!
    Sets the fade \a duration used for visual effects.
*/
void AbstractButton::setFadeDuration(float duration) {
    m_fadeDuration = duration;
}
/*!
    Returns the color used when the button is highlighted.
*/
Vector4 AbstractButton::highlightedColor() const {
    return m_highlightedColor;
}
/*!
    Sets the \a color used when the button is highlighted.
*/
void AbstractButton::setHighlightedColor(const Vector4 color) {
    m_highlightedColor = color;
}
/*!
    Returns the normal color of the button.
*/
Vector4 AbstractButton::normalColor() const {
    return m_normalColor;
}
/*!
    Sets the normal \a color of the button.
*/
void AbstractButton::setNormalColor(const Vector4 color) {
    m_normalColor = color;
    Frame *back = background();
    if(back) {
        back->setColor(m_normalColor);
    }
}
/*!
    Returns the color used when the button is pressed.
*/
Vector4 AbstractButton::pressedColor() const {
    return m_pressedColor;
}
/*!
    Sets the \a color used when the button is pressed.
*/
void AbstractButton::setPressedColor(const Vector4 color) {
    m_pressedColor = color;
}
/*!
    Returns true if the button is checkable; otherwise, false.
*/
bool AbstractButton::isCheckable() const {
    return m_checkable;
}
/*!
    Sets whether the button is \a checkable.
*/
void AbstractButton::setCheckable(bool checkable) {
    m_checkable = checkable;
}
/*!
    Returns true if the button is checked; otherwise, false.
*/
bool AbstractButton::isChecked() const {
    return m_checked;
}
/*!
    Sets the \a checked state of the button.
*/
void AbstractButton::setChecked(bool checked) {
    m_checked = checked;
    checkStateSet();

    emitSignal(_SIGNAL(toggled(bool)), m_checked);
}
/*!
    Returns true if the button is in exclusive mode; otherwise, false.
*/
bool AbstractButton::isExclusive() const {
    return m_exclusive;
}
/*!
    Sets whether the button is in \a exclusive mode.
*/
void AbstractButton::setExclusive(bool exclusive) {
    m_exclusive = exclusive;
}
/*!
    Returns true if the button is mirrored; otherwise, false.
*/
bool AbstractButton::isMirrored() const {
    return m_mirrored;
}
/*!
    Sets whether the button should be \a mirrored.
*/
void AbstractButton::setMirrored(bool mirrored) {
    m_mirrored = mirrored;
}
/*!
    \internal
    Internal method called to update the button's visual appearance and handle user interaction.
*/
void AbstractButton::update() {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }
    Vector4 color(m_normalColor);

    Frame *back = background();
    bool hover = (back) ? back->rectTransform()->isHovered(pos.x, pos.y) : rectTransform()->isHovered(pos.x, pos.y);
    if(m_hovered != hover) {
        m_currentFade = 0.0f;
        m_hovered = hover;
    }

    if(m_hovered) {
        color = m_highlightedColor;
        if(Input::isMouseButtonDown(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;

            if(m_checkable) {
                setChecked(!m_checked);
            }
            emitSignal(_SIGNAL(pressed()));
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_ENDED)) {
            m_currentFade = 0.0f;

            emitSignal(_SIGNAL(clicked()));
        }

        if(Input::isMouseButton(Input::MOUSE_LEFT) || Input::touchCount() > 0) {
            color = m_pressedColor;
        }
    }

    if(back) {
        if(m_currentFade < 1.0f) {
            m_currentFade += 1.0f / m_fadeDuration * Timer::deltaTime();
            m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

            back->setColor(MIX(back->color(), color, m_currentFade));
        }
    }

    Widget::update();
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void AbstractButton::applyStyle() {
    Widget::applyStyle();

    // Background color
    auto it = m_styleRules.find("background-color");
    if(it != m_styleRules.end()) {
        setNormalColor(StyleSheet::toColor(it->second.second));
    }
}
/*!
    \internal
    Internal method to handle the button's checked state, ensuring exclusivity in exclusive mode.
*/
void AbstractButton::checkStateSet() {
    if(m_exclusive && m_checked) {
        for(auto it : actor()->getChildren()) {
            Actor *actor = dynamic_cast<Actor *>(it);
            if(actor) {
                AbstractButton *btn = static_cast<AbstractButton *>(actor->component("AbstractButton"));
                if(btn && btn != this) {
                    btn->setChecked(false);
                }
            }
        }
    }
}
/*!
    \internal
    Internal method called to compose the button component by adding background, label, and icon components.
*/
void AbstractButton::composeComponent() {
    Widget::composeComponent();

    // Add background
    Actor *background = Engine::composeActor(gFrameClass, "Background", actor());
    Frame *frame = background->getComponent<Frame>();
    frame->setCorners(Vector4(gCorner));

    setBackground(frame);

    // Add label
    Actor *text = Engine::composeActor(gLabelClass, gLabelClass, actor());
    Label *label = text->getComponent<Label>();
    label->setAlign(Alignment::Middle | Alignment::Center);
    label->setColor(m_textColor);

    RectTransform *r = label->rectTransform();
    r->setSize(rectTransform()->size());
    r->setAnchors(Vector2(0.0f), Vector2(1.0f));

    setLabel(label);
    setText("Text");

    // Add icon
    Actor *icon = Engine::composeActor(gImageClass, gImageClass, actor());
    Image *image = icon->getComponent<Image>();

    setIcon(image);

    rectTransform()->setSize(Vector2(100.0f, 30.0f));
}
