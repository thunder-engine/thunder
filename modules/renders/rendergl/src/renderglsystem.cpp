#include "renderglsystem.h"

#include "agl.h"

#include "analytics/profiler.h"

#include "apipeline.h"
#include "adeferredshading.h"

#include "components/aspritegl.h"
#include "components/acameragl.h"
#include "components/astaticmeshgl.h"
#include "components/adirectlightgl.h"

#include "components/scene.h"

#include <log.h>

RenderGLSystem::RenderGLSystem(Engine *engine) :
        m_pPipeline(nullptr),
        IRenderSystem(engine) {
    PROFILER_MARKER;

    ATextureGL::registerClassFactory();
    AMaterialGL::registerClassFactory();
    AMeshGL::registerClassFactory();

    ASpriteGL::registerClassFactory();

    ACameraGL::registerClassFactory();
    AStaticMeshGL::registerClassFactory();
    ADirectLightGL::registerClassFactory();
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

#if (_WIN32)
    //uint32_t err    = glewInit();
    //if(err != GLEW_OK) {
    //    Log(Log::ERR) << "[ Render::RenderGLSystem ]" << glewGetErrorString(err);
    //    return false;
    //}
#endif

#if !defined(__ANDROID__)
    if(!gladLoadGL() /*!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)*/) {
        Log(Log::ERR) << "[ Render::RenderGLSystem ] Failed to initialize OpenGL context";
        return false;
    }
#endif

    glDepthFunc     (GL_LEQUAL);
    glEnable        (GL_DEPTH_TEST);

    int32_t targets = 1;
#ifndef GL_ES_VERSION_2_0
    glEnable        (GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glGetIntegerv	(GL_MAX_DRAW_BUFFERS, &targets);
#endif
    if(targets >= ADeferredShading::G_TARGETS) {
        m_pPipeline = new ADeferredShading(m_pEngine);
    } else {
        m_pPipeline = new APipeline(m_pEngine);
    }
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

void RenderGLSystem::clearRenderTarget(bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILER_MARKER;

    uint32_t flags  = 0;
    if(clearColor) {
        flags   |= GL_COLOR_BUFFER_BIT;
        glClearColor(color.x, color.y, color.z, color.w);
    }
    if(clearDepth) {
        flags   |= GL_DEPTH_BUFFER_BIT;
        glClearDepthf(depth);
    }
    glClear(flags);
}

void RenderGLSystem::drawMesh(const Matrix4 &model, Mesh *mesh, uint32_t surface, uint8_t layer, MaterialInstance *material) {
    PROFILER_MARKER;

    if(m_pPipeline) {
        m_pPipeline->drawMesh(model, mesh, surface, layer, material);
    }
}

void RenderGLSystem::setColor(const Vector4 &color) {
    PROFILER_MARKER;

    if(m_pPipeline) {
        m_pPipeline->setColor(color);
    }
}

void RenderGLSystem::setCamera(const Camera &camera) {
    PROFILER_MARKER;

    if(m_pPipeline) {
        m_pPipeline->cameraSet(camera);
    }
}

void RenderGLSystem::setRenderTarget(uint8_t numberColors, const Texture *colors, uint8_t numberDepth, const Texture *depth) {

}
