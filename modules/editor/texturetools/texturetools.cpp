#include "texturetools.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "converter/textureconverter.h"
#include "editor/textureedit.h"

static const char *meta = \
"{"
"   \"module\": \"TextureTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Texture Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"TextureConverter\": \"converter\","
"       \"TextureEdit\": \"editor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new TextureTools(engine);
}

TextureTools::TextureTools(Engine *engine) :
        Module(engine) {
}

const char *TextureTools::metaInfo() const {
    return meta;
}

void *TextureTools::getObject(const char *name) {
    if(strcmp(name, "TextureConverter") == 0) {
        return new TextureConverter;
    } else if(strcmp(name, "TextureEdit") == 0) {
        return new TextureEdit;
    }
    return nullptr;
}
