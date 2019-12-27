#include "rendergl.h"

#include "renderglsystem.h"

Module *moduleCreate(Engine *engine) {
    return new RenderGL(engine);
}

RenderGL::RenderGL(Engine *engine) :
        m_pEngine(engine),
        m_pSystem(new RenderGLSystem(engine)) {
}

RenderGL::~RenderGL() {
    delete m_pSystem;
}

const char *RenderGL::description() const {
    return "OpenGL Render Module";
}

const char *RenderGL::version() const {
    return "1.0";
}

uint8_t RenderGL::types() const {
    return SYSTEM;
}

System *RenderGL::system() {
    return m_pSystem;
}
