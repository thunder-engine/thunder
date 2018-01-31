#include "agl.h"

#if !(GL_ES_VERSION_2_0)

#include "adeferredshading.h"

#include <amath.h>
#include <log.h>
#include <atools.h>

#include "components/scene.h"

#include "components/aspritegl.h"

#include "analytics/profiler.h"

#include "postprocess/aambientocclusiongl.h"

#define OVERRIDE "texture0"

ADeferredShading::ADeferredShading(Engine *engine) :
        APipeline(engine),
        m_Depth() {

    m_HDR       = false;

    buffers     = new GLenum[G_TARGETS];
    m_pGBuffer  = new ATextureGL[G_TARGETS];

    // G buffer
    glGenFramebuffers(1, &fb_g_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_g_id);

    m_pGBuffer[G_NORMALS].create (GL_TEXTURE_2D, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2);
    m_pGBuffer[G_DIFFUSE].create (GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT);
    m_pGBuffer[G_PARAMS].create  (GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT);
    m_pGBuffer[G_EMISSIVE].create(GL_TEXTURE_2D, GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT);

    for(int i = 0; i < G_TARGETS; i++) {
        buffers[i]		= GL_COLOR_ATTACHMENT0 + i;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_pGBuffer[i].id(), 0);
    }
    m_Depth.create          (GL_TEXTURE_2D, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    glFramebufferTexture2D  (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Depth.id(), 0);

    // Summary pass buffer
    glGenFramebuffers       (1, &fb_s_id);
    glBindFramebuffer       (GL_FRAMEBUFFER, fb_s_id);
    glFramebufferTexture2D  (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Depth.id(), 0);

    glBindFramebuffer       (GL_FRAMEBUFFER, 0);

}

ADeferredShading::~ADeferredShading() {
    glDeleteFramebuffers(1, &fb_s_id);

    glDeleteFramebuffers(1, &fb_g_id);

    delete []buffers;
    delete []m_pGBuffer;
}

void ADeferredShading::draw(Scene &scene, uint32_t resource) {
    APipeline::draw(scene, resource);
    // Fill G buffer pass
    glBindFramebuffer   (GL_FRAMEBUFFER, fb_g_id);

    glReadBuffer    ( GL_COLOR_ATTACHMENT0 );
    glDrawBuffer    ( GL_COLOR_ATTACHMENT0 );
    glDrawBuffers   ( G_TARGETS, buffers );
    glClear         ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // Draw Opaque pass
    drawComponents(scene, IDrawObjectGL::DEFAULT);
    // Screen Space Ambient Occlusion effect
    ATextureGL *t   = &m_pGBuffer[G_EMISSIVE];//&(m_pAO->draw(m_pGBuffer[G_EMISSIVE], *this));

    glBindFramebuffer( GL_FRAMEBUFFER, fb_s_id );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pGBuffer[G_EMISSIVE].id(), 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Depth.id(), 0 );
    // Light pass
    glActiveTexture (GL_TEXTURE0);
    m_pGBuffer[G_NORMALS].bind();

    glActiveTexture (GL_TEXTURE1);
    m_pGBuffer[G_DIFFUSE].bind();

    glActiveTexture (GL_TEXTURE2);
    m_pGBuffer[G_PARAMS].bind();

    glActiveTexture (GL_TEXTURE3);
    m_pGBuffer[G_EMISSIVE].bind();

    glActiveTexture (GL_TEXTURE4);
    m_Depth.bind();

    glDisable(GL_DEPTH_TEST);

    updateLights(scene, IDrawObjectGL::LIGHT);

    glEnable(GL_DEPTH_TEST);

    glActiveTexture (GL_TEXTURE4);
    m_Depth.unbind();

    glActiveTexture (GL_TEXTURE3);
    m_pGBuffer[G_EMISSIVE].unbind();

    glActiveTexture (GL_TEXTURE2);
    m_pGBuffer[G_PARAMS].unbind();

    glActiveTexture (GL_TEXTURE1);
    m_pGBuffer[G_DIFFUSE].unbind();

    glActiveTexture (GL_TEXTURE0);
    m_pGBuffer[G_NORMALS].unbind();

    cameraReset();

    // Draw Transparent pass
    drawComponents(scene, IDrawObjectGL::TRANSLUCENT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    makeOrtho();

    // Final pass
    if(resource != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, resource);
    }

    m_pSprite->setTexture(postProcess(m_pGBuffer[G_EMISSIVE]));
    m_pSprite->draw(*this, IDrawObjectGL::UI);

    //glBlendFunc     (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //drawComponents(scene, IDrawObjectGL::UI);
}

void ADeferredShading::resize(int32_t width, int32_t height) {
    APipeline::resize(width, height);

    for(int i = 0; i < G_TARGETS; i++) {
        m_pGBuffer[i].resize(m_Screen.x, m_Screen.y);
    }

    m_Depth.resize(m_Screen.x, m_Screen.y);
}

#endif
