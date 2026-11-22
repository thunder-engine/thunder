#include "pipelinetools.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "converter/pipelineconverter.h"
#include "editor/legacypipelineedit.h"

static const char *meta = \
"{"
"   \"module\": \"PipelineTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Pipeline Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"PipelineConverter\": \"converter\","
"       \"LegacyPipelineEdit\": \"legacyeditor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new PipelineTools(engine);
}

PipelineTools::PipelineTools(Engine *engine) :
        Module(engine) {
}

const char *PipelineTools::metaInfo() const {
    return meta;
}

void *PipelineTools::getObject(const char *name) {
    if(strcmp(name, "PipelineConverter") == 0) {
        return new PipelineConverter;
    } else if(strcmp(name, "LegacyPipelineEdit") == 0) {
        return new LegacyPipelineEdit;
    }
    return nullptr;
}
