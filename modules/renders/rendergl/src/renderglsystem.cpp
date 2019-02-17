#include "renderglsystem.h"

#include "agl.h"

#include <components/camera.h>
#include <components/scene.h>

#include <resources/pipeline.h>

#include <analytics/profiler.h>

#include "resources/atexturegl.h"
#include "resources/arendertexturegl.h"

#include "commandbuffergl.h"

#include <log.h>

void _CheckGLError(const char* file, int line) {
    GLenum err ( glGetError() );

    while ( err != GL_NO_ERROR ) {
        std::string error;
        switch ( err ) {
            case GL_INVALID_OPERATION:  error="GL_INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:       error="GL_INVALID_ENUM";           break;
            case GL_INVALID_VALUE:      error="GL_INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:      error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="GL_INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        Log(Log::DBG) << error.c_str() <<" - " << file << ":" << line;;
        err = glGetError();
    }
    return;
}

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
        CheckGLError();
        Log(Log::ERR) << "[ Render::RenderGLSystem ] Failed to initialize OpenGL context";
        return false;
    }
    glEnable        (GL_TEXTURE_CUBE_MAP_SEAMLESS);
    CheckGLError();
#endif

    int32_t targets;
    glGetIntegerv	(GL_MAX_DRAW_BUFFERS, &targets);
    CheckGLError();

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
