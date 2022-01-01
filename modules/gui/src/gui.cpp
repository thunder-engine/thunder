#include <amath.h>
#include <engine.h>

#include "gui.h"

#include "systems/guisystem.h"

static const char *meta = \
"{"
"   \"module\": \"Gui\","
"   \"version\": \"1.0\","
"   \"description\": \"GUI Plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"GuiSystem\": \"system\""
"   },"
"   \"components\": ["
"       \"AbstractButton\","
"       \"Button\","
"       \"Image\","
"       \"Label\","
"       \"ProgressBar\","
"       \"RectTransform\","
"       \"Switch\","
"       \"Widget\""
"   ]"
"}";

Module *moduleCreate(Engine *engine) {
    return new Gui(engine);
}

Gui::Gui(Engine *engine) :
        Module(engine),
        m_system(new GuiSystem) {
}

Gui::~Gui() {
    delete m_system;
}

const char *Gui::metaInfo() const {
    return meta;
}

void *Gui::getObject(const char *) {
    return m_system;
}
