#include "renderglsystem.h"

#include "agl.h"

#include <components/camera.h>
#include <components/world.h>

#include "resources/meshgl.h"
#include "resources/texturegl.h"
#include "resources/materialgl.h"
#include "resources/rendertargetgl.h"
#include "resources/computebuffergl.h"
#include "resources/computeshadergl.h"

#include "systems/resourcesystem.h"

#include <pipelinecontext.h>
#include "commandbuffergl.h"

#include <log.h>

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
        aDebug() << error << " - " << file << ":" << line;
        err = glGetError();
    }
    return;
}

RenderGLSystem::RenderGLSystem() :
        RenderSystem(),
        m_target(-1) {

    PROFILE_FUNCTION();

    if(registered == 0) {
        ResourceSystem *system = Engine::resourceSystem();

        TextureGL::registerClassFactory(system);
        RenderTargetGL::registerClassFactory(system);
        MaterialGL::registerClassFactory(system);
        MeshGL::registerClassFactory(system);
        ComputeBufferGL::registerClassFactory(system);
        ComputeShaderGL::registerClassFactory(system);

        CommandBufferGL::registerClassFactory(system);
    }
    ++registered;
}

RenderGLSystem::~RenderGLSystem() {
    PROFILE_FUNCTION();

    --registered;
    if(registered == 0) {
        ResourceSystem *system = Engine::resourceSystem();

        TextureGL::unregisterClassFactory(system);
        RenderTargetGL::unregisterClassFactory(system);
        MaterialGL::unregisterClassFactory(system);
        MeshGL::unregisterClassFactory(system);
        ComputeBufferGL::unregisterClassFactory(system);
        ComputeShaderGL::unregisterClassFactory(system);

        CommandBufferGL::unregisterClassFactory(system);
    }

    setName("RenderGL");
}
/*!
    Initialization of render.
*/
bool RenderGLSystem::init() {
    PROFILE_FUNCTION();

    static bool done = false;
    if(!done) {
#ifndef THUNDER_MOBILE
        if(!gladLoadGL()) {
            CheckGLError();
            aWarning() << "[ RenderGL ] Failed to initialize OpenGL context.";
            return false;
        }
        CheckGLError();
#endif

        int32_t texture;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture);
        CheckGLError();

        Texture::setMaxTextureSize(texture);

        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &texture);
        CheckGLError();

        Texture::setMaxCubemapSize(texture);

        CommandBufferGL::setInited();

        done = true;
    }

    return RenderSystem::init();
}
/*!
    Main drawing procedure.
*/
void RenderGLSystem::update(World *world) {
    PROFILE_FUNCTION();

    PipelineContext *context = pipelineContext();
    if(context && CommandBufferGL::isInited()) {
        CommandBufferGL *cmd = static_cast<CommandBufferGL *>(context->buffer());
        cmd->begin();

        if(m_target == -1) {
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_target);
            static_cast<RenderTargetGL *>(context->defaultTarget())->setNativeHandle(m_target);
        }

        RenderSystem::update(world);
    }
}

#if defined(SHARED_DEFINE)
#include "editor/rhiwrapper.h"

QWindow *RenderGLSystem::createRhiWindow(Viewport *viewport) {
    return createWindow(viewport);
}
#endif
