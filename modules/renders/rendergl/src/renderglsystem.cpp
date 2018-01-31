#include "renderglsystem.h"

#include "agl.h"

#include "analytics/profiler.h"

#include "apipeline.h"
#include "adeferredshading.h"

#include "components/aspritegl.h"
#include "components/acameragl.h"
#include "components/astaticmeshgl.h"
#include "components/alightsourcegl.h"

#include "components/scene.h"

#include <log.h>

RenderGLSystem::RenderGLSystem(Engine *engine) :
        m_pPipeline(nullptr),
        IRenderSystem(engine) {
    PROFILER_MARKER

    ATextureGL::registerClassFactory();
    AMaterialGL::registerClassFactory();
    AMeshGL::registerClassFactory();

    ASpriteGL::registerClassFactory();

    ACameraGL::registerClassFactory();
    AStaticMeshGL::registerClassFactory();
    ALightSourceGL::registerClassFactory();
}

RenderGLSystem::~RenderGLSystem() {
    PROFILER_MARKER

    delete m_pPipeline;
}

/*!
    Initialization of render.
*/
bool RenderGLSystem::init() {
    PROFILER_MARKER

#if (_WIN32)
    //uint32_t err    = glewInit();
    //if(err != GLEW_OK) {
    //    Log(Log::ERR) << "[ Render::RenderGLSystem ]" << glewGetErrorString(err);
    //    return false;
    //}
#endif

    if(!gladLoadGL() /*!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)*/) {
        Log(Log::ERR) << "[ Render::RenderGLSystem ] Failed to initialize OpenGL context";
        return false;
    }

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
    PROFILER_MARKER

    if(m_pPipeline) {
        m_pPipeline->draw(scene, resource);
    }
}

void RenderGLSystem::overrideController(IController *controller) {
    PROFILER_MARKER

    if(m_pPipeline) {
        m_pPipeline->overrideController(controller);
    }
}

void RenderGLSystem::drawStrip(const Matrix4 &model, const Vector3List &points, bool line) {
    PROFILER_MARKER

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, &points[0]);
    glDrawArrays(line ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, 0, points.size());
    glDisableVertexAttribArray(0);
}

void RenderGLSystem::setColor(const Vector4 &color) {
    PROFILER_MARKER

    glColor4fv(color.v);
    if(m_pPipeline) {
        m_pPipeline->setColor(color);
    }
}

