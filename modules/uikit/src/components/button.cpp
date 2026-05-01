#include "components/button.h"

#include "components/frame.h"
#include "components/recttransform.h"

/*!
    \class Button
    \brief The Button class represents a push button class.
    \inmodule Gui

    The Button class represents a push button element in a graphical user interface (GUI).
    It is a fundamental UI component that allows users to trigger actions or commands through a simple click or press.
*/

Button::Button() {

}
/*!
    Sets the icon \a image component associated with the button.
*/
void Button::setImage(Image *image) {
    AbstractButton::setImage(image);

    if(image) {
        RectTransform *rect = image->rectTransform();
        if(rect) {
            rect->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
            rect->setPivot(Vector2(0.0f, 0.5f));
            if(label()) {
                rect->setPosition(Vector3(8.0f, 0.0f, 0.0f));
            }
        }
    }
}
/*!
    \internal
    Internal method called to compose the button component by adding background, label, and icon components.
*/
void Button::composeComponent() {
    AbstractButton::composeComponent();

    Frame *back = background();
    if(back) {
        back->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
    }
}
