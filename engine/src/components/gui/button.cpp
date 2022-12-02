#include "components/gui/button.h"

#include "components/gui/label.h"
#include "components/gui/frame.h"
#include "components/gui/recttransform.h"

#include "components/actor.h"
#include "components/textrender.h"

#include "resources/font.h"

namespace  {
    const char *gLabel = "Label";
}

void Button::composeComponent() {
    AbstractButton::composeComponent();

    Frame *back = background();
    if(back) {
        back->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
    }
}
