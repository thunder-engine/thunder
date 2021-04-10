#include "renderrt.h"

#include "renderrtsystem.h"

Module *moduleCreate(Engine *engine) {
    return new RenderRT(engine);
}

RenderRT::RenderRT(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new RenderRTSystem(engine)) {
}

RenderRT::~RenderRT() {
    delete m_pSystem;
}

const char *RenderRT::description() const {
    return "Raytracing Render Module";
}

const char *RenderRT::version() const {
    return "1.0";
}

uint8_t RenderRT::types() const {
    return SYSTEM;
}

System *RenderRT::system() {
    return m_pSystem;
}
