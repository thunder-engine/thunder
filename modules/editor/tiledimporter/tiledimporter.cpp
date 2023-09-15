#include "tiledimporter.h"

#include <engine.h>

#include <cstring>

#include "converter/tiledmapconverter.h"
#include "converter/tiledsetconverter.h"

static const char *meta = \
"{"
"   \"module\": \"TiledImporter\","
"   \"version\": \"1.0\","
"   \"description\": \"Tiled Importer plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"TiledSetConverter\": \"converter\","
"       \"TiledMapConverter\": \"converter\","
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new TiledImporter(engine);
}

TiledImporter::TiledImporter(Engine *engine) :
        Module(engine) {
}

const char *TiledImporter::metaInfo() const {
    return meta;
}

void *TiledImporter::getObject(const char *name) {
    if(strcmp(name, "TiledMapConverter") == 0) {
        return new TiledMapConverter;
    } else if(strcmp(name, "TiledSetConverter") == 0) {
        return new TiledSetConverter;
    }
    return nullptr;
}
