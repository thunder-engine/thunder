#include "components/button.h"

#include "components/label.h"
#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/font.h>

void Button::composeComponent() {
    AbstractButton::composeComponent();

    // Add label
    Actor *text = Engine::composeActor("Label", "Text", actor());

    RectTransform *rect = dynamic_cast<RectTransform *>(text->transform());
    if(rect) {
        rect->setMinAnchors(Vector2(0.0f, 0.0f));
        rect->setMaxAnchors(Vector2(1.0f, 1.0f));
    }

    RectTransform *parent = dynamic_cast<RectTransform *>(actor()->transform());
    if(parent) {
        parent->setSize(Vector2(100.0f, 30.0f));
    }
}
