#include "components/button.h"

#include "components/label.h"
#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/font.h>

namespace  {
    const char *LABEL = "Label";
}

void Button::composeComponent() {
    AbstractButton::composeComponent();

    // Add label
    Actor *text = Engine::composeActor(LABEL, "Text", actor());
    Label *label = static_cast<Label *>(text->component(LABEL));
    label->setParent(text);

    RectTransform *parent = dynamic_cast<RectTransform *>(actor()->transform());
    if(parent) {
        parent->setSize(Vector2(100.0f, 30.0f));
    }

    RectTransform *rect = dynamic_cast<RectTransform *>(text->transform());
    if(rect) {
        rect->setSize(Vector2(1.0f, 1.0f));
        rect->setMinAnchors(Vector2(0.0f, 0.0f));
        rect->setMaxAnchors(Vector2(1.0f, 1.0f));
    }
}
