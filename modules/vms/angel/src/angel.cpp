#include "angel.h"

#include "angelsystem.h"

#ifdef NEXT_SHARED
#include "converters/angelbuilder.h"

Module *moduleCreate(Engine *engine) {
    return new Angel(engine);
}
#endif

Angel::Angel(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new AngelSystem(engine)) {
}

Angel::~Angel() {
    delete m_pSystem;
}

const char *Angel::description() const {
    return "AngelScript Module";
}

const char *Angel::version() const {
    return "1.0";
}

int Angel::types() const {
    int result  = SYSTEM;
#ifdef NEXT_SHARED
    result  |= CONVERTER;
#endif
    return result;
}

System *Angel::system() {
    return m_pSystem;
}

IConverter *Angel::converter() {
#ifdef NEXT_SHARED
    return new AngelBuilder(m_pSystem);
#endif
    return nullptr;
}
