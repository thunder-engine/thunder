#include "motiontools.h"

#include <engine.h>

#include <cstring>

#include "converter/animationbuilder.h"
#include "editor/animationedit.h"

static const char *meta = \
"{"
"   \"module\": \"MotionTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Tiled Importer plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"AnimationBuilderSettings\": \"converter\","
"       \"AnimationEdit\": \"editor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new MotionTools(engine);
}

MotionTools::MotionTools(Engine *engine) :
        Module(engine) {
}

const char *MotionTools::metaInfo() const {
    return meta;
}

void *MotionTools::getObject(const char *name) {
    if(strcmp(name, "AnimationBuilderSettings") == 0) {
        return new AnimationControllerBuilder;
    } else if(strcmp(name, "AnimationEdit") == 0) {
        return new AnimationEdit;
    }
    return nullptr;
}
