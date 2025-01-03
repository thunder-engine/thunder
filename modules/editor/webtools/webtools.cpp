#include "webtools.h"

#include <engine.h>

#include "converter/emscriptenbuilder.h"

static const char *meta = \
"{"
"   \"module\": \"WebTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Web Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"EmscriptenBuilder\": \"converter\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new WebTools(engine);
}

WebTools::WebTools(Engine *engine) :
        Module(engine) {
}

const char *WebTools::metaInfo() const {
    return meta;
}

void *WebTools::getObject(const char *) {
    static EmscriptenBuilder *builder = nullptr;
    if(builder == nullptr) {
        builder = new EmscriptenBuilder;
    }
    return builder;
}
