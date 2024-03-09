#include "spineimporter.h"

#include <engine.h>

#include <cstring>

#include "converter/spineconverter.h"

static const char *meta = \
"{"
"   \"module\": \"SpineImporter\","
"   \"version\": \"1.0\","
"   \"description\": \"Spine 2D Importer plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"SpineConverter\": \"converter\","
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new SpineImporter(engine);
}

SpineImporter::SpineImporter(Engine *engine) :
        Module(engine) {
}

const char *SpineImporter::metaInfo() const {
    return meta;
}

void *SpineImporter::getObject(const char *name) {
    if(strcmp(name, "SpineConverter") == 0) {
        return new SpineConverter;
    }
    return nullptr;
}
