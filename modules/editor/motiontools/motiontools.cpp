#include "motiontools.h"

#include <engine.h>

#include <cstring>

#include "converter/animationbuilder.h"

#include "legacyeditor/editor/animationedit.h"
#include "legacyeditor/property/conditionedit.h"
#include "legacyeditor/gadget/timelineedit.h"

static const char *meta = \
"{"
"   \"module\": \"MotionTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Animation Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"AnimationBuilderSettings\": \"converter\","
"       \"LegacyAnimationEdit\": \"legacyeditor\","
"       \"LegacyCondition\": \"legacyproperty\","
"       \"TimelineEdit\": \"legacygadget\""
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
    } else if(strcmp(name, "LegacyAnimationEdit") == 0) {
        return new LegacyAnimationEdit;
    } else if(strcmp(name, "LegacyCondition") == 0) {
        return new LegacyConditionEdit;
    } else if(strcmp(name, "TimelineEdit") == 0) {
        return new TimelineEdit;
    }
    return nullptr;
}
