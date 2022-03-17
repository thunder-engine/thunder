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
        Module(engine),
        m_pSystem(nullptr) {
}

RenderVK::~RenderVK() {
    delete m_pSystem;
}

const char *RenderVK::metaInfo() const {
    return meta;
}

void *RenderVK::getObject(const char *) {
    if(m_pSystem == nullptr) {
        m_pSystem = new RenderVkSystem(m_engine);
    }
    return m_pSystem;
}
