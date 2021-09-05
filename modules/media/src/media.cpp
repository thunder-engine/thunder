#include "media.h"

#include "mediasystem.h"

#ifdef NEXT_SHARED
#include "converters/audioconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Media(engine);
}
#endif

Media::Media(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new MediaSystem()){
}

Media::~Media() {
    delete m_pSystem;
}

const char *Media::description() const {
    return "Media Module";
}

const char *Media::version() const {
    return "1.0";
}

uint32_t Media::types() const {
    uint32_t result = SYSTEM;
#ifdef NEXT_SHARED
    result  |= CONVERTER;
#endif
    return result;
}

System *Media::system() {
    return m_pSystem;
}

IConverter *Media::converter() {
#ifdef NEXT_SHARED
    return new AudioConverter();
#endif
    return nullptr;
}
