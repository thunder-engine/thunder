#include "rendervk.h"

#include "rendervksystem.h"

#ifdef NEXT_SHARED
Module *moduleCreate(Engine *engine) {
    return new RenderVK(engine);
}
#endif

RenderVK::RenderVK(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new RenderVkSystem(engine)) {
}

RenderVK::~RenderVK() {
    delete m_pSystem;
}

const char *RenderVK::description() const {
    return "Vulkan Render Module";
}

const char *RenderVK::version() const {
    return "1.0";
}

uint8_t RenderVK::types() const {
    return SYSTEM;
}

System *RenderVK::system() {
    return m_pSystem;
}
