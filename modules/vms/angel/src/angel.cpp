#include "angel.h"

#include "angelsystem.h"

#ifdef NEXT_SHARED
#include "converters/angelbuilder.h"

Module *moduleCreate(Engine *engine) {
    return new Angel(engine);
}
#endif

static const char *meta = \
"{"
"   \"version\": \"1.0\","
"   \"module\": \"Angel\","
"   \"description\": \"AngelScript Module\","
"   \"systems\": ["
"       \"AngelSystem\""
"   ],"
"   \"converters\": ["
"       \"AngelBuilder\""
"   ],"
"   \"extensions\": ["
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

System *Angel::system(const char *) {
    return m_pSystem;
}
#ifdef NEXT_SHARED
AssetConverter *Angel::assetConverter(const char *) {
    return new AngelBuilder(m_pSystem);
}
#endif
