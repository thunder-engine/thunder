#include "rendervk.h"

#include "rendervksystem.h"

#ifdef SHARED_DEFINE
Module *moduleCreate(Engine *engine) {
    return new RenderVK(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"RenderVK\","
"   \"version\": \"1.0\","
"   \"description\": \"Vulkan Render Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"RenderVK\": \"render\""
"   }"
"}";

RenderVK::RenderVK(Engine *engine) :
        Module(engine) {
}

RenderVK::~RenderVK() {

}

const char *RenderVK::metaInfo() const {
    return meta;
}

void *RenderVK::getObject(const char *) {
    return new RenderVkSystem(m_engine);
}
