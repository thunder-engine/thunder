#include "particletools.h"

#include <amath.h>
#include <engine.h>

#include <cstring>

#include "converter/effectconverter.h"
#include "editor/particleedit.h"

static const char *meta = \
"{"
"   \"module\": \"ParticleTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Particle Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"EffectConverter\": \"converter\","
"       \"ParticleEdit\": \"editor\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new ParticleTools(engine);
}

ParticleTools::ParticleTools(Engine *engine) :
        Module(engine) {
}

const char *ParticleTools::metaInfo() const {
    return meta;
}

void *ParticleTools::getObject(const char *name) {
    if(strcmp(name, "EffectConverter") == 0) {
        return new EffectConverter;
    } else if(strcmp(name, "ParticleEdit") == 0) {
        return new ParticleEdit;
    }
    return nullptr;
}
