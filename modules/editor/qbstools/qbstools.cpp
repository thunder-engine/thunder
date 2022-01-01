#include "qbstools.h"

#include <engine.h>

#include "converter/qbsbuilder.h"

static const char *meta = \
"{"
"   \"module\": \"QbsTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Qbs Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"QbsBuilder\": \"converter\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new QbsTools(engine);
}

QbsTools::QbsTools(Engine *engine) :
        Module(engine) {
}

const char *QbsTools::metaInfo() const {
    return meta;
}

void *QbsTools::getObject(const char *) {
    static QbsBuilder *builder = nullptr;
    if(builder == nullptr) {
        builder = new QbsBuilder;
    }
    return builder;
}
