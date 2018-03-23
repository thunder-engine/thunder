#include "agl.h"

#if !(GL_ES_VERSION_2_0)

#include "adeferredshading.h"

#include <amath.h>
#include <log.h>
#include <tools.h>

#include "components/scene.h"
#include "components/camera.h"

#include "components/aspritegl.h"

#include "analytics/profiler.h"

#include "postprocess/aambientocclusiongl.h"

ADeferredShading::ADeferredShading(Engine *engine) :
        APipeline(engine) {

    m_HDR       = false;

    Material *mtl   = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    if(mtl) {
        m_pSprite   = mtl->createInstance();
    }
    m_pPlane    = Engine::loadResource<Mesh>(".embedded/plane.fbx");

    m_pGBuffer  = new ATextureGL[G_TARGETS];

    m_pGBuffer[G_NORMALS] .create(GL_TEXTURE_2D, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2);
    m_pGBuffer[G_DIFFUSE] .create(GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT);
    m_pGBuffer[G_PARAMS]  .create(GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT);
    m_pGBuffer[G_EMISSIVE].create(GL_TEXTURE_2D, GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT);

    m_DepthMap            .create(GL_TEXTURE_2D, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

    // G buffer
    glGenFramebuffers(1, &fb_g_id);
    // Summary pass buffer
    glGenFramebuffers(1, &fb_s_id);
}

ADeferredShading::~ADeferredShading() {
    glDeleteFramebuffers(1, &fb_s_id);
    glDeleteFramebuffers(1, &fb_g_id);

    delete []m_pGBuffer;
}

void ADeferredShading::draw(Scene &scene, uint32_t resource) {
    APipeline::draw(scene, resource);

    Camera *camera  = activeCamera();
    // Fill G buffer pass
    glBindFramebuffer(GL_FRAMEBUFFER, fb_g_id);
    m_Buffer->setRenderTarget(G_TARGETS, m_pGBuffer, &m_Depth);
    m_Buffer->clearRenderTarget(true, ((camera) ? camera->color() : Vector4(0.0)), false);
    glDepthFunc(GL_EQUAL);

    cameraReset();
    // Draw Opaque pass
    drawComponents(scene, ICommandBuffer::DEFAULT);

    analizeScene(scene);

    // Screen Space Ambient Occlusion effect
    ATextureGL &t   = m_pGBuffer[G_EMISSIVE];//&(m_pAO->draw(m_pGBuffer[G_EMISSIVE], *this));

    glBindFramebuffer( GL_FRAMEBUFFER, fb_s_id );
    m_Buffer->setRenderTarget(1, &m_pGBuffer[G_EMISSIVE], &m_Depth);

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

    updateLights(scene, ICommandBuffer::LIGHT);

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
    drawComponents(scene, ICommandBuffer::TRANSLUCENT);

    glBindFramebuffer(GL_FRAMEBUFFER, resource);
    m_Buffer->setViewProjection(Matrix4(), Matrix4::ortho(0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));

    m_pSprite->setTexture("texture0", postProcess(t));
    m_Buffer->drawMesh(Matrix4(), m_pPlane, 0, ICommandBuffer::UI, m_pSprite);

    //glBlendFunc   (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //drawComponents(scene, IDrawObjectGL::UI);
}

void ADeferredShading::resize(uint32_t width, uint32_t height) {
    APipeline::resize(width, height);

    for(int i = 0; i < G_TARGETS; i++) {
        m_pGBuffer[i].resize(m_Screen.x, m_Screen.y);
    }
    m_DepthMap.resize(m_Screen.x, m_Screen.y);
}

#endif
