#include "uikit.h"

#include <cstring>

#include "uisystem.h"

#ifdef SHARED_DEFINE
#include "editor/uiedit.h"

Module *moduleCreate(Engine *engine) {
    return new UiKit(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"UiKit\","
"   \"version\": \"1.0\","
"   \"description\": \"UiKit Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"UiSystem\": \"system\","
"       \"UiEdit\": \"editor\""
"   },"
"   \"components\": ["
"       \"AbstractButton\","
"       \"Button\","
"       \"FloatInput\","
"       \"Frame\","
"       \"Label\","
"       \"Menu\","
"       \"ProgressBar\","
"       \"RectTransform\","
"       \"Switch\","
"       \"TextInput\","
"       \"ToolButton\","
"       \"Widget\""
"   ]"
"}";

UiKit::UiKit(Engine *engine) :
        Module(engine),
        m_system(nullptr) {

}

UiKit::~UiKit() {
    delete m_system;
}

const char *UiKit::metaInfo() const {
    return meta;
}

void *UiKit::getObject(const char *name) {
    if(strcmp(name, "UiSystem") == 0) {
        if(m_system == nullptr) {
            m_system = new UiSystem;
        }
        return m_system;
    }
#ifdef SHARED_DEFINE
    if(strcmp(name, "UiEdit") == 0) {
        return new UiEdit;
    }
#endif
    return nullptr;
}
