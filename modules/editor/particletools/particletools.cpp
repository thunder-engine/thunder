#include "particletools.h"

#include <amath.h>
#include <engine.h>
#include <editor/propertyedit.h>

#include <cstring>

#include "converter/effectbuilder.h"
#include "editor/particleedit.h"

#include "property/SelectorEdit.h"

static const char *meta = \
"{"
"   \"module\": \"ParticleTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Particle Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"EffectConverter\": \"converter\","
"       \"EffectBuilder\": \"converter\","
"       \"ParticleEdit\": \"editor\""
"   }"
"}";

PropertyEdit *createCustomEditor(int userType, QWidget *parent, const QString &name, QObject *object) {
    PropertyEdit *result = nullptr;

    if(userType == qMetaTypeId<SelectorData>()) {
        result = new SelectorEdit(parent);
    }

    return result;
}

Module *moduleCreate(Engine *engine) {
    return new ParticleTools(engine);
}

ParticleTools::ParticleTools(Engine *engine) :
        Module(engine) {

    PropertyEdit::registerEditorFactory(createCustomEditor);
}

const char *ParticleTools::metaInfo() const {
    return meta;
}

void *ParticleTools::getObject(const char *name) {
    if(strcmp(name, "EffectBuilder") == 0) {
        return new EffectBuilder;
    } else if(strcmp(name, "ParticleEdit") == 0) {
        return new ParticleEdit;
    }
    return nullptr;
}
