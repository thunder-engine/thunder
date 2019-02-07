#include "renderglsystem.h"

#include "agl.h"

#include <components/camera.h>
#include <components/scene.h>

#include <resources/pipeline.h>

#include <analytics/profiler.h>
#include <log.h>

#include "resources/atexturegl.h"
#include "resources/arendertexturegl.h"

#include "commandbuffergl.h"

RenderGLSystem::RenderGLSystem() :
        ISystem() {
    PROFILER_MARKER;

    ATextureGL::registerClassFactory(this);
    ARenderTextureGL::registerClassFactory(this);
    AMaterialGL::registerClassFactory(this);
    AMeshGL::registerClassFactory(this);

    CommandBufferGL::registerClassFactory(this);
}

RenderGLSystem::~RenderGLSystem() {
    PROFILER_MARKER;

    ObjectSystem system;

    ATextureGL::unregisterClassFactory(this);
    ARenderTextureGL::unregisterClassFactory(this);
    AMaterialGL::unregisterClassFactory(this);
    AMeshGL::unregisterClassFactory(this);

    CommandBufferGL::unregisterClassFactory(this);
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

    return true;
}

const char *RenderGLSystem::name() const {
    return "RenderGL";
}

/*!
    Main drawing procedure.
*/
void RenderGLSystem::update(Scene *scene) {
    PROFILER_MARKER;

    PROFILER_RESET(POLYGONS);
    PROFILER_RESET(DRAWCALLS);

    Camera *camera  = Camera::current();
    if(camera) {
        Pipeline *pipe  = camera->pipeline();
        pipe->combineComponents(scene, true);
        pipe->draw(scene, *camera);
    }
}
