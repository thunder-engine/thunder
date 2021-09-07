#include "rendergl.h"

#include "renderglsystem.h"

#ifdef NEXT_SHARED
Module *moduleCreate(Engine *engine) {
    return new RenderGL(engine);
}
#endif

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

int RenderGL::types() const {
    return SYSTEM;
}

System *RenderGL::system() {
    return m_pSystem;
}
