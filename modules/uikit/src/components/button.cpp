#include "components/button.h"

#include "components/label.h"
#include "components/image.h"
#include "components/frame.h"
#include "components/recttransform.h"

/*!
    \class Button
    \brief The Button class represents a push button class.
    \inmodule Gui

    The Button class represents a push button element in a graphical user interface (GUI).
    It is a fundamental UI component that allows users to trigger actions or commands through a simple click or press.
*/

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
