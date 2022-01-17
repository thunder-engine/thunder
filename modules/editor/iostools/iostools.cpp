#include "iostools.h"

#include <engine.h>

#include "converter/xcodebuilder.h"

static const char *meta = \
"{"
"   \"module\": \"iOSTools\","
"   \"version\": \"1.0\","
"   \"description\": \"iOS Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"XcodeBuilder\": \"converter\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new IosTools(engine);
}

IosTools::IosTools(Engine *engine) :
        Module(engine) {
}

const char *IosTools::metaInfo() const {
    return meta;
}

void *IosTools::getObject(const char *) {
    static XcodeBuilder *builder = nullptr;
    if(builder == nullptr) {
        builder = new XcodeBuilder;
    }
    return builder;
}
