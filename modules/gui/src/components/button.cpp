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

    Label *label = static_cast<Label *>(text->component("Label"));
    label->setFontSize(14);
    label->setColor(Vector4(0.20f, 0.20f, 0.20f, 1.0f));
    label->setAlign(Alignment::Center | Alignment::Middle);
    label->setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    label->setText("Button");
}
