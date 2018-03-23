#include "components/adirectlightgl.h"

#include "agl.h"
#include "commandbuffergl.h"
#include "apipeline.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/camera.h"

#include "resources/amaterialgl.h"

#include "filters/ablurgl.h"

#define MAX_LODS 1
#define SM_RESOLUTION_DEFAULT 512

ADirectLightGL::ADirectLightGL() {
    m_pMatrix       = nullptr;
    m_Resolution    = SM_RESOLUTION_DEFAULT;
    m_CSM           = true;
    m_LOD           = Vector4(8.0f, 32.0f, 128.0f, 512.0f);
    m_pPlane        = Engine::loadResource<AMeshGL>(".embedded/plane.fbx");

    m_LODCount      = (m_CSM) ? MAX_LODS : 1;
    m_pMatrix       = new Matrix4[m_LODCount];
    m_pDepthMap     = new ATextureGL[m_LODCount];
    // LOD distance
    for(int i = 0; i < m_LODCount; i++) {
        m_Distance[i]   = m_LOD[i];
        m_pDepthMap[i].create(GL_TEXTURE_2D, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
        m_pDepthMap[i].resize(m_Resolution, m_Resolution);
    }
    m_pMaterial     = Engine::loadResource<AMaterialGL>(".embedded/VSM.mtl");
    m_pMaterialInstance = m_pMaterial->createInstance();

    glGenFramebuffers(1, &m_DepthBuffer);
}

ADirectLightGL::~ADirectLightGL() {
    glDeleteFramebuffers(1, &m_DepthBuffer);
}

void ADirectLightGL::draw(APipeline &pipeline, uint8_t layer) {
    glActiveTexture(GL_TEXTURE5);
    m_pDepthMap[0].bind();

    if(m_pMaterial) {
        AMaterialGL *mtl    = static_cast<AMaterialGL *>(m_pMaterial);
        if(mtl->bind(m_pMaterialInstance, ICommandBuffer::DEFAULT, AMaterialGL::Static)) {
            uint32_t program    = mtl->getProgram(AMaterialGL::Static);
            int location    = glGetUniformLocation(program, "layer0");
            if(location > -1) {
                glUniform1i(location, 0);
            }
            location    = glGetUniformLocation(program, "layer1");
            if(location > -1) {
                glUniform1i(location, 1);
            }
            location    = glGetUniformLocation(program, "layer2");
            if(location > -1) {
                glUniform1i(location, 2);
            }
            location    = glGetUniformLocation(program, "layer3");
            if(location > -1) {
                glUniform1i(location, 3);
            }
            location    = glGetUniformLocation(program, "depthMap");
            if(location > -1) {
                glUniform1i(location, 4);
            }
            location    = glGetUniformLocation(program, "shadowMap");
            if(location > -1) {
                glUniform1i(location, 5);
            }

            Actor &a    = actor();

            location	= glGetUniformLocation(program, "light.pos");
            if(location > -1) {
                glUniform3fv(location, 1, a.position().v);
            }
            location	= glGetUniformLocation(program, "light.dir");
            if(location > -1) {
                glUniform3fv(location, 1, (a.rotation() * Vector3(0.0f, 0.0f, 1.0f)).v);
            }
            location	= glGetUniformLocation(program, "light.color");
            if(location > -1) {
                glUniform3fv(location, 1, m_Color.v);
            }
            location	= glGetUniformLocation(program, "light.brightness");
            if(location > -1) {
                glUniform1f(location, m_Brightness);
            }
            location	= glGetUniformLocation(program, "light.shadows");
            if(location > -1) {
                glUniform1f(location, (m_Shadows) ? 1.0f : 0.0f );
            }
            if(m_Shadows) {
                location    = glGetUniformLocation(program, "light.lod");
                if(location > -1) {
                    glUniform4fv(location, 1, m_Distance.v);
                }
                location    = glGetUniformLocation(program, "light.matrix");
                if(location > -1) {
                    for(uint8_t layer = 0; layer < m_LODCount; layer++) {
                        glUniformMatrix4fv(location + layer, 1, false, m_pMatrix[layer].mat);
                    }
                }
                location    = glGetUniformLocation(program, "light.mvpi");
                if(location > -1) {
                    Camera *camera  = pipeline.activeCamera();
                    Matrix4 mv, p;
                    camera->matrices(mv, p);
                    Matrix4 mvpi    = (p * mv).inverse();
                    glUniformMatrix4fv(location, 1, GL_FALSE, mvpi.mat);
                }
            }
            mtl->unbind(ICommandBuffer::DEFAULT);
        }
        ICommandBuffer *b   = pipeline.buffer();
        b->setViewProjection(Matrix4(), Matrix4::ortho( 0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
        b->drawMesh(Matrix4(), m_pPlane, 0, layer, m_pMaterialInstance);
    }

    glActiveTexture(GL_TEXTURE5);
    m_pDepthMap[0].unbind();
}

void ADirectLightGL::shadowsUpdate(APipeline &pipeline) {
    if(isEnable() && m_Shadows) {
        Matrix4 view    = Matrix4(actor().rotation().toMatrix()).inverse();
        Matrix4 model;
        model.translate(-pipeline.activeCamera()->actor().position());
        Matrix4 mv      = view * model;
        for(uint32_t lod = 0; lod < m_LODCount; lod++) {
            float size      = m_LOD[lod];
            Matrix4 proj    = Matrix4::ortho(-size, size,-size, size,-size, size);

            Matrix4 mat     = Matrix4(Vector3(0.5f), Quaternion(), Vector3(0.5f));

            mat *= proj;
            mat *= view;
            mat *= model;

            m_pMatrix[lod]  = mat;
            // Draw in the depth buffer from position of the light source
            ICommandBuffer *b   = pipeline.buffer();
            b->setViewProjection(mv, proj);
            b->setViewport(0, 0, m_Resolution, m_Resolution);
            glBindFramebuffer(GL_FRAMEBUFFER, m_DepthBuffer);
            b->setRenderTarget(0, nullptr, &m_pDepthMap[lod]);
            b->clearRenderTarget();
            pipeline.drawComponents(*(pipeline.scene()), ICommandBuffer::SHADOWCAST);
        }
    }
}
