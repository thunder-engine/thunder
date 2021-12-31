#include "rendergl.h"

#include "renderglsystem.h"

#ifdef NEXT_SHARED
Module *moduleCreate(Engine *engine) {
    return new RenderGL(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"RenderGL\","
"   \"version\": \"1.0\","
"   \"description\": \"OpenGL Render Module\","
"   \"objects\": {"
"       \"RenderGLSystem\": \"system\""
"   }"
"}";

RenderGL::RenderGL(Engine *engine) :
        Module(engine),
        m_pSystem(new RenderGLSystem(engine)) {
}

RenderGL::~RenderGL() {
    delete m_pSystem;
}

const char *RenderGL::metaInfo() const {
    return meta;
}

void *RenderGL::getObject(const char *) {
    return m_pSystem;
}
