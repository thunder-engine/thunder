#include "renderglsystem.h"

#include "agl.h"

#include <components/camera.h>
#include <components/scene.h>

#include <resources/pipeline.h>

#include "resources/texturegl.h"
#include "resources/rendertargetgl.h"

#include "commandbuffergl.h"

#include <log.h>

#define MAX_RESOLUTION 8192

void _CheckGLError(const char* file, int line) {
    GLenum err ( glGetError() );

    while(err != GL_NO_ERROR) {
        std::string error;
        switch ( err ) {
            case GL_INVALID_OPERATION:  error="GL_INVALID_OPERATION"; break;
            case GL_INVALID_ENUM:       error="GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:      error="GL_INVALID_VALUE"; break;
            case GL_OUT_OF_MEMORY:      error="OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        Log(Log::DBG) << error.c_str() <<" - " << file << ":" << line;
        err = glGetError();
    }
    return;
}

RenderGLSystem::RenderGLSystem(Engine *engine) :
        RenderSystem(),
        m_pEngine(engine) {

    PROFILE_FUNCTION();

    System *system = m_pEngine->resourceSystem();

    TextureGL::registerClassFactory(system);
    RenderTargetGL::registerClassFactory(system);
    MaterialGL::registerClassFactory(system);
    MeshGL::registerClassFactory(system);

    CommandBufferGL::registerClassFactory(m_pEngine);
}

RenderGLSystem::~RenderGLSystem() {
    PROFILE_FUNCTION();

    System *system = m_pEngine->resourceSystem();

    TextureGL::unregisterClassFactory(system);
    RenderTargetGL::unregisterClassFactory(system);
    MaterialGL::unregisterClassFactory(system);
    MeshGL::unregisterClassFactory(system);

    CommandBufferGL::unregisterClassFactory(m_pEngine);
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
    setAtlasPageSize(texture, texture);

    CommandBufferGL::setInited();

    return true;
}
/*!
    Main drawing procedure.
*/
void RenderGLSystem::update(Scene *scene) {
    PROFILE_FUNCTION();

    Camera *camera = Camera::current();
    if(camera && CommandBufferGL::isInited()) {
        int32_t target;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target);

        Pipeline *pipe = camera->pipeline();
        CommandBufferGL *cmd = static_cast<CommandBufferGL *>(pipe->buffer());
        cmd->begin();

        static_cast<RenderTargetGL *>(pipe->defaultTarget())->setNativeHandle(target);

        RenderSystem::update(scene);
    }
}

#if defined(NEXT_SHARED)
#include "editor/rhiwrapper.h"

QWindow *RenderGLSystem::createRhiWindow() const {
    return createWindow();
}

vector<uint8_t> RenderGLSystem::renderOffscreen(Scene *scene, int width, int height) {
    makeCurrent();

    static RenderTargetGL *target = nullptr;
    if(target == nullptr) {
        target = static_cast<RenderTargetGL *>(Engine::objectCreate<RenderTarget>());

        Texture *color = Engine::objectCreate<Texture>();
        color->setFormat(Texture::RGBA8);
        color->setWidth(width);
        color->setHeight(height);

        Texture *depth = Engine::objectCreate<Texture>();
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
        depth->setWidth(width);
        depth->setHeight(height);

        target->setColorAttachment(0, color);
        target->setDepthAttachment(depth);
    }

    Camera *camera = Camera::current();
    if(camera) {
        Pipeline *pipe = camera->pipeline();
        pipe->resize(width, height);
    }
    target->bindBuffer(0);

    auto result = RenderSystem::renderOffscreen(scene, width, height);

    result.resize(width * height * 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, result.data());

    return result;
}
#endif
