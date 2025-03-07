#include "rendermt.h"

#include "rendermtsystem.h"

#ifdef SHARED_DEFINE
Module *moduleCreate(Engine *engine) {
    return new RenderMT(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"RenderMT\","
"   \"version\": \"1.0\","
"   \"description\": \"Metal Render Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"RenderMT\": \"render\""
"   }"
"}";

RenderMT::RenderMT(Engine *engine) :
        Module(engine) {
}

RenderMT::~RenderMT() {

}

const char *RenderMT::metaInfo() const {
    return meta;
}

void *RenderMT::getObject(const char *) {
    return new RenderMtSystem(m_engine);
}
