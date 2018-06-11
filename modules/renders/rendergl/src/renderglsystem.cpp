#include "renderglsystem.h"

#include "agl.h"

#include "analytics/profiler.h"

#include "pipeline.h"

#include "components/adirectlightgl.h"
#include "components/scene.h"

#include "resources/arendertexturegl.h"

#include "commandbuffergl.h"

#include <log.h>

RenderGLSystem::RenderGLSystem(Engine *engine) :
        m_pPipeline(nullptr),
        ISystem(engine) {
    PROFILER_MARKER;

    ATextureGL::registerClassFactory();
    ARenderTextureGL::registerClassFactory();
    AMaterialGL::registerClassFactory();
    AMeshGL::registerClassFactory();

    ADirectLightGL::registerClassFactory();

    CommandBufferGL::registerClassFactory();
}

RenderGLSystem::~RenderGLSystem() {
    PROFILER_MARKER;

    delete m_pPipeline;
}

/*!
    Initialization of render.
*/
bool RenderGLSystem::init() {
    PROFILER_MARKER;

#ifndef THUNDER_MOBILE
    if(!gladLoadGL()) {
        Log(Log::ERR) << "[ Render::RenderGLSystem ] Failed to initialize OpenGL context";
        return false;
    }
    glEnable        (GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

    int32_t targets;
    glGetIntegerv	(GL_MAX_DRAW_BUFFERS, &targets);

    m_pPipeline = new APipeline(m_pEngine);

    return true;
}

const char *RenderGLSystem::name() const {
    return "RenderGL";
}

/*!
    Main drawing procedure.
*/
void RenderGLSystem::update(Scene &scene, uint32_t resource) {
    PROFILER_MARKER;

    PROFILER_RESET(POLYGONS);
    PROFILER_RESET(DRAWCALLS);

    if(m_pPipeline) {
        m_pPipeline->draw(scene, resource);
    }
}

void RenderGLSystem::overrideController(IController *controller) {
    PROFILER_MARKER;

    if(m_pPipeline) {
        m_pPipeline->overrideController(controller);
    }
}

void RenderGLSystem::resize(uint32_t width, uint32_t height) {
    PROFILER_MARKER;

    if(m_pPipeline) {
        m_pPipeline->resize(width, height);
    }
}
