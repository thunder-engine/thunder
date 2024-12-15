#include "components/button.h"

#include "components/label.h"
#include "components/image.h"
#include "components/frame.h"
#include "components/recttransform.h"

/*!
    \class Button
    \brief The Button class represents a push button class.
    \inmodule Gui
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
