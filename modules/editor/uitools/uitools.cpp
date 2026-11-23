#include "uitools.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "editor/legacyuiedit.h"
#include "converters/stylesheetconverter.h"
#include "converters/uiconverter.h"

static const char *meta = \
"{"
"   \"module\": \"UiTools\","
"   \"version\": \"1.0\","
"   \"description\": \"UI Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"StyleSheetConverter: \"converter\","
"       \"UiConverter: \"converter\","
"       \"LegacyUiEdit\": \"legacyeditor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new UiTools(engine);
}

UiTools::UiTools(Engine *engine) :
        Module(engine) {
}

const char *UiTools::metaInfo() const {
    return meta;
}

void *UiTools::getObject(const char *name) {
    if(strcmp(name, "StyleSheetConverter") == 0) {
        return new StyleSheetConverter;
    } else if(strcmp(name, "UiConverter") == 0) {
        return new UiConverter;
    } else if(strcmp(name, "LegacyUiEdit") == 0) {
        return new LegacyUiEdit;
    }
    return nullptr;
}
