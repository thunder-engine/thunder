#include "media.h"

#include "mediasystem.h"

#ifdef NEXT_SHARED
#include "converters/audioconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Media(engine);
}
#endif

static const char *meta = \
"{"
"   \"version\": \"1.0\","
"   \"description\": \"Media Module\","
"   \"systems\": ["
"       \"MediaSystem\""
"   ],"
"   \"converters\": ["
"       \"AudioConverter\""
"   ]"
"}";

Media::Media(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new MediaSystem()){
}

Media::~Media() {
    delete m_pSystem;
}

const char *Media::metaInfo() const {
    return meta;
}

System *Media::system(const char *) {
    return m_pSystem;
}
#ifdef NEXT_SHARED
AssetConverter *Media::assetConverter(const char *) {
    return new AudioConverter();
}
#endif
