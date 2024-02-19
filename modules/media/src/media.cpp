#include "media.h"

#include "mediasystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "converters/audioconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Media(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"Media\","
"   \"version\": \"1.0\","
"   \"description\": \"Media Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"MediaSystem\": \"system\","
"       \"AudioConverter\": \"converter\""
"   },"
"   \"components\": ["
"       \"AudioSource\""
"   ]"
"}";

Media::Media(Engine *engine) :
        Module(engine),
        m_system(nullptr) {
}

Media::~Media() {
    delete m_system;
}

const char *Media::metaInfo() const {
    return meta;
}

void *Media::getObject(const char *name) {
    if(strcmp(name, "MediaSystem") == 0) {
        if(m_system == nullptr) {
            m_system = new MediaSystem();
        }
        return m_system;
    }
#ifdef SHARED_DEFINE
    else if(strcmp(name, "AudioConverter") == 0) {
        return new AudioConverter();
    }
#endif
    return nullptr;
}
