#include "media.h"

#include "mediasystem.h"

#include <cstring>

#ifdef NEXT_SHARED
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
        m_pSystem(new MediaSystem()){
}

Media::~Media() {
    delete m_pSystem;
}

const char *Media::metaInfo() const {
    return meta;
}

void *Media::getObject(const char *name) {
    if(strcmp(name, "MediaSystem") == 0) {
        return m_pSystem;
    }
#ifdef NEXT_SHARED
    else if(strcmp(name, "AudioConverter") == 0) {
        return new AudioConverter();
    }
#endif
    return nullptr;
}
