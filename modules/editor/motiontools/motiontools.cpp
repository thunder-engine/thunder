#include "motiontools.h"

#include <engine.h>

#include <cstring>

#include "converter/animationbuilder.h"
#include "editor/animationedit.h"

#include "editor/property/conditionedit.h"

static const char *meta = \
"{"
"   \"module\": \"MotionTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Animation Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"AnimationBuilderSettings\": \"converter\","
"       \"AnimationEdit\": \"editor\","
"       \"Condition\": \"property\""
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
    } else if(strcmp(name, "Condition") == 0) {
        return new ConditionEdit;
    }
    return nullptr;
}
