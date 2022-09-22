#include "components/button.h"

#include "components/label.h"
#include "components/frame.h"
#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/font.h>

namespace  {
    const char *gLabel = "Label";
}

void Button::composeComponent() {
    AbstractButton::composeComponent();

    RectTransform *rect = rectTransform();
    if(rect) {
        background()->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
        label()->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));

        rect->setSize(Vector2(100.0f, 30.0f));
    }
}
