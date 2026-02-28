#include "uikit.h"

#include <cstring>

#include "uisystem.h"

#ifdef SHARED_DEFINE
#include "editor/uiedit.h"
#include "converters/uiconverter.h"
#include "converters/stylesheetconverter.h"

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
"       \"UiEdit\": \"editor\","
"       \"UiConverter\": \"converter\","
"       \"StyleSheetConverter\": \"converter\""
"   },"
"   \"components\": ["
"       \"AbstractButton\","
"       \"AbstractSlider\","
"       \"Button\","
"       \"CheckBox\","
"       \"FloatInput\","
"       \"Foldout\","
"       \"Frame\","
"       \"Image\","
"       \"Label\","
"       \"Menu\","
"       \"ProgressBar\","
"       \"RectTransform\","
"       \"ScrollBar\","
"       \"Slider\","
"       \"Splitter\","
"       \"Switch\","
"       \"TextInput\","
"       \"ToolButton\","
"       \"UiLoader\","
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
    } else if(strcmp(name, "UiConverter") == 0) {
        return new UiConverter;
    } else if(strcmp(name, "StyleSheetConverter") == 0) {
        return new StyleSheetConverter;
    }
#endif
    return nullptr;
}
