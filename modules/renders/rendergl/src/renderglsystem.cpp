#include "renderglsystem.h"

#include "agl.h"

#include <components/camera.h>
#include <components/scenegraph.h>

#include "resources/meshgl.h"
#include "resources/texturegl.h"
#include "resources/materialgl.h"
#include "resources/rendertargetgl.h"

#include "systems/resourcesystem.h"

#include <pipelinecontext.h>
#include "commandbuffergl.h"

#include <log.h>

#define MAX_RESOLUTION 8192

static int32_t registered = 0;

void _CheckGLError(const char* file, int line) {
    GLenum err ( glGetError() );

    while(err != GL_NO_ERROR) {
        std::string error;
        switch ( err ) {
            case GL_INVALID_OPERATION: error="GL_INVALID_OPERATION"; break;
            case GL_INVALID_ENUM:      error="GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:     error="GL_INVALID_VALUE"; break;
            case GL_OUT_OF_MEMORY:     error="OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error="GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        Log(Log::DBG) << error.c_str() <<" - " << file << ":" << line;
        err = glGetError();
    }
    return;
}

RenderGLSystem::RenderGLSystem(Engine *engine) :
        RenderSystem(),
        m_engine(engine) {

    PROFILE_FUNCTION();

    if(registered == 0) {
        System *system = Engine::resourceSystem();

        TextureGL::registerClassFactory(system);
        RenderTargetGL::registerClassFactory(system);
        MaterialGL::registerClassFactory(system);
        MeshGL::registerClassFactory(system);

        CommandBufferGL::registerClassFactory(m_engine);
    }
    ++registered;
}

RenderGLSystem::~RenderGLSystem() {
    PROFILE_FUNCTION();

    --registered;
    if(registered == 0) {
        System *system = Engine::resourceSystem();

        TextureGL::unregisterClassFactory(system);
        RenderTargetGL::unregisterClassFactory(system);
        MaterialGL::unregisterClassFactory(system);
        MeshGL::unregisterClassFactory(system);

        CommandBufferGL::unregisterClassFactory(m_engine);
    }
}

const char *RenderGLSystem::name() const {
    return "RenderGL";
}
/*!
    Initialization of render.
*/
bool RenderGLSystem::init() {
    PROFILE_FUNCTION();

#ifndef THUNDER_MOBILE
    if(!gladLoadGL()) {
        CheckGLError();
        aWarning() << "[ RenderGL ] Failed to initialize OpenGL context.";
        return false;
    }
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    CheckGLError();
#endif

    int32_t targets;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &targets);
    CheckGLError();

    int32_t attribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribs);

    int32_t texture;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture);
    CheckGLError();

    texture = MIN(texture, MAX_RESOLUTION);

    bool result = RenderSystem::init();

    pipelineContext()->setShadowPageSize(texture, texture);

    CommandBufferGL::setInited();

    return result;
}
/*!
    Main drawing procedure.
*/
void RenderGLSystem::update(SceneGraph *graph) {
    PROFILE_FUNCTION();

    PipelineContext *context = pipelineContext();
    if(context && CommandBufferGL::isInited()) {
        CommandBufferGL *cmd = static_cast<CommandBufferGL *>(context->buffer());
        cmd->begin();

        if(!isOffscreenMode()) {
            int32_t target;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target);
            static_cast<RenderTargetGL *>(context->defaultTarget())->setNativeHandle(target);
        }

        RenderSystem::update(graph);
    }
}

#if defined(SHARED_DEFINE)
#include "editor/rhiwrapper.h"

QWindow *RenderGLSystem::createRhiWindow() const {
    return createWindow();
}

ByteArray RenderGLSystem::renderOffscreen(SceneGraph *sceneGraph, int width, int height) {
    makeCurrent();
    return RenderSystem::renderOffscreen(sceneGraph, width, height);
}

#endif

