#include "shadertools.h"

#include <amath.h>
#include <engine.h>

#include "converter/shaderbuilder.h"
#include "editor/materialedit.h"

static const char *meta = \
"{"
"   \"module\": \"ShaderTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Shader Tools plugin\","
"   \"objects\": {"
"       \"ShaderBuilder\": \"converter\","
"       \"MaterialEdit\": \"editor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new ShaderTools(engine);
}

ShaderTools::ShaderTools(Engine *engine) :
        Module(engine) {
}

const char *ShaderTools::metaInfo() const {
    return meta;
}

void *ShaderTools::getObject(const char *name) {
    if(strcmp(name, "ShaderBuilder") == 0) {
        return new ShaderBuilder;
    } else if(strcmp(name, "MaterialEdit") == 0) {
        return new MaterialEdit;
    }
    return nullptr;
}
