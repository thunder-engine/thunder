#include "rendergl.h"

#include "renderglsystem.h"

#ifdef SHARED_DEFINE
Module *moduleCreate(Engine *engine) {
    return new RenderGL(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"RenderGL\","
"   \"version\": \"1.0\","
"   \"description\": \"OpenGL Render Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"RenderGL\": \"render\""
"   }"
"}";

RenderGL::RenderGL(Engine *engine) :
        Module(engine),
        m_pSystem(nullptr) {
}

RenderGL::~RenderGL() {
    delete m_pSystem;
}

const char *RenderGL::metaInfo() const {
    return meta;
}

void *RenderGL::getObject(const char *) {
    if(m_pSystem == nullptr) {
        m_pSystem = new RenderGLSystem(m_engine);
    }
    return m_pSystem;
}
