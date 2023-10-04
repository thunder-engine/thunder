#include "timeline.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "editor/timelineedit.h"

static const char *meta = \
"{"
"   \"module\": \"Timeline\","
"   \"version\": \"1.0\","
"   \"description\": \"Animation Track editor\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"TimelineEdit\": \"gadget\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new TimeLine(engine);
}

TimeLine::TimeLine(Engine *engine) :
        Module(engine) {
}

const char *TimeLine::metaInfo() const {
    return meta;
}

void *TimeLine::getObject(const char *name) {
    return new TimelineEdit;
}
