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

RenderGLSystem::RenderGLSystem(Engine *engine) :
        IRenderSystem(engine) {
    PROFILER_MARKER

    m_pPipeline = NULL;

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
    @return 0               Intialization successful.
    @return -1              Intialization error ("Unsupported extentions").
*/
bool RenderGLSystem::init() {
    PROFILER_MARKER

#if (_WIN32)
    glewInit();
#endif

    glDepthFunc     (GL_LEQUAL);
    glEnable        (GL_DEPTH_TEST);

    glEnable        (GL_TEXTURE_CUBE_MAP_SEAMLESS);

    int32_t targets;
    glGetIntegerv	(GL_MAX_DRAW_BUFFERS, &targets);
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

void RenderGLSystem::drawBillboard(const Vector3 &position, const Vector2 &size, Texture &image) {
    PROFILER_MARKER

    if(m_pPipeline) {
        Matrix4 result;
        Matrix4 m;
        m.translate(position);
        result *= m;
        m.scale(Vector3(size, 1.0));
        result *= m;

        m_pPipeline->setTransform(result);
        //AMaterialGL *mat    = m_pPipeline->materialSprite();
        //mat->overrideTexture("texture0", &image);

        //mat->bind(*m_pPipeline, IDrawObjectGL::TRANSLUCENT, AMaterialGL::Billboard);
        //m_pPipeline->drawQuad();
        //mat->unbind(IDrawObjectGL::TRANSLUCENT);

        m_pPipeline->resetTransform();
    }
}

void RenderGLSystem::drawPath(const Vector3List &points) {
    PROFILER_MARKER

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &points[0]);

    glDrawArrays(GL_LINE_STRIP, 0, points.size());

    glDisableClientState(GL_VERTEX_ARRAY);
}

void RenderGLSystem::setColor(const Vector4 &color) {
    PROFILER_MARKER

    if(m_pPipeline) {
        glColor4fv(color.v);
        //m_pPipeline->setColor(color);
    }
}

