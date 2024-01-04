#include "angel.h"

#include "angelsystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "converters/angelbuilder.h"

Module *moduleCreate(Engine *engine) {
    return new Angel(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"Angel\","
"   \"version\": \"1.0\","
"   \"description\": \"Angel Script Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"AngelSystem\": \"system\","
"       \"AngelBuilder\": \"converter\""
"   },"
"   \"components\": ["
"       \"AngelBehaviour\""
"   ]"
"}";

Angel::Angel(Engine *engine) :
        Module(engine),
        m_pSystem(new AngelSystem(engine)) {
}

Angel::~Angel() {
    delete m_pSystem;
}

const char *Angel::metaInfo() const {
    return meta;
}

void *Angel::getObject(const char *name) {
    if(strcmp(name, "AngelSystem") == 0) {
        return m_pSystem;
    }
#ifdef SHARED_DEFINE
    else if(strcmp(name, "AngelBuilder") == 0) {
        static AngelBuilder *builder = nullptr;
        if(builder == nullptr) {
            builder = new AngelBuilder(m_pSystem);
        }
        return builder;
    }
#endif
    return nullptr;
}
