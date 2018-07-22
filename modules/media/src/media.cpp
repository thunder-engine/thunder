#include "media.h"

#include "mediasystem.h"
#ifdef BUILD_SHARED
    #include "converters/audioconverter.h"
#endif
IModule *moduleCreate(Engine *engine) {
    return new Media(engine);
}

Media::Media(Engine *engine) :
        m_pEngine(engine) {
}

Media::~Media() {

}

const char *Media::description() const {
    return "Media Module";
}

const char *Media::version() const {
    return "1.0";
}

uint8_t Media::types() const {
    uint8_t result  = SYSTEM;
#ifdef BUILD_SHARED
    result  |= CONVERTER;
#endif
    return result;
}

ISystem *Media::system() {
    return new MediaSystem(m_pEngine);
}

IConverter *Media::converter() {
#ifdef BUILD_SHARED
    return new AudioConverter();
#endif
    return nullptr;
}
