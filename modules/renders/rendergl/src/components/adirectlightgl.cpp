#include "components/adirectlightgl.h"

#include <float.h>

#include "agl.h"
#include "commandbuffergl.h"
#include "apipeline.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/camera.h"

#include "resources/amaterialgl.h"

#include "filters/ablurgl.h"

#define MAX_LODS 4
#define SM_RESOLUTION_DEFAULT 1024

ADirectLightGL::ADirectLightGL() {
    m_pMatrix       = nullptr;
    m_Resolution    = SM_RESOLUTION_DEFAULT;
    m_CSM           = true;
    m_pPlane        = Engine::loadResource<AMeshGL>(".embedded/plane.fbx");

    m_LODCount      = (m_CSM) ? MAX_LODS : 1;
    m_pMatrix       = new Matrix4[m_LODCount];
    m_pTiles        = new Vector4[m_LODCount];

    m_DepthMap.create(GL_TEXTURE_2D, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
    m_DepthMap.resize(m_Resolution * 2, m_Resolution * 2);

    m_pMaterial     = Engine::loadResource<AMaterialGL>(".embedded/VSM.mtl");
    m_pMaterialInstance = m_pMaterial->createInstance();
}

void ADirectLightGL::draw(APipeline &pipeline, uint8_t layer) {
    glActiveTexture(GL_TEXTURE5);
    m_DepthMap.bind();

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
                location    = glGetUniformLocation(program, "light.bias");
                if(location > -1) {
                    glUniform1f(location, 0.001f);
                }

                location    = glGetUniformLocation(program, "light.lod");
                if(location > -1) {
                    glUniform3fv(location, 1, m_NormalizedDistance.v);
                }
                location    = glGetUniformLocation(program, "light.matrix");
                if(location > -1) {
                    for(uint8_t layer = 0; layer < m_LODCount; layer++) {
                        glUniformMatrix4fv(location + layer, 1, false, m_pMatrix[layer].mat);
                    }
                }
                location    = glGetUniformLocation(program, "light.tiles");
                if(location > -1) {
                    for(uint8_t layer = 0; layer < m_LODCount; layer++) {
                        glUniform4fv(location + layer, 1, m_pTiles[layer].v);
                    }
                }

                location    = glGetUniformLocation(program, "light.mvpi");
                if(location > -1) {
                    Camera *camera  = pipeline.activeCamera();
                    Matrix4 mv, p;
                    camera->matrices(mv, p);

                    glUniformMatrix4fv(location, 1, GL_FALSE, (p * mv).inverse().mat);
                }
            }
            mtl->unbind(ICommandBuffer::DEFAULT);
        }
        ICommandBuffer *b   = pipeline.buffer();
        b->setViewProjection(Matrix4(), Matrix4::ortho( 0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
        b->drawMesh(Matrix4(), m_pPlane, 0, layer, m_pMaterialInstance);
    }

    glActiveTexture(GL_TEXTURE5);
    m_DepthMap.unbind();
}

void ADirectLightGL::shadowsUpdate(APipeline &pipeline) {
    ICommandBuffer *b   = pipeline.buffer();
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMap.buffer());
    b->setRenderTarget(0, nullptr, &m_DepthMap);
    b->clearRenderTarget();

    if(isEnable() && m_Shadows) {
        Camera *camera  = pipeline.activeCamera();
        Matrix4 mv, p;
        camera->matrices(mv, p);
        {
            float split     = 0.9f;
            float nearPlane = camera->nearPlane();
            float farPlane  = camera->farPlane();
            for(int i = 1; i <= m_LODCount; i++) {
                float f = (float)i / (float)m_LODCount;
                float l = nearPlane * pow(farPlane / nearPlane, f);
                float u = nearPlane + (farPlane - nearPlane) * f;
                float v = MIX(u, l, split);
                m_Distance[i - 1]   = v;
                Vector4 depth       = p * Vector4(0.0f, 0.0f, -v, 1.0f);
                m_NormalizedDistance[i - 1] = (depth.z / depth.w);
            }
        }

        float nearPlane = camera->nearPlane();
        Matrix4 view    = Matrix4(actor().rotation().toMatrix()).inverse();
        Matrix4 inv     = mv.inverse();
        for(uint32_t lod = 0; lod < m_LODCount; lod++) {
            float dist  = m_Distance[lod];
            Vector3 bb[2]   = {Vector3(FLT_MAX), Vector3(-FLT_MAX)};
            for(Vector3 &it : camera->frustumCorners(nearPlane, dist)) {
                Vector3 pos = (inv * it);
                bb[0].x = MIN(bb[0].x, pos.x);
                bb[0].y = MIN(bb[0].y, pos.y);
                bb[0].z = MIN(bb[0].z, pos.z);

                bb[1].x = MAX(bb[1].x, pos.x);
                bb[1].y = MAX(bb[1].y, pos.y);
                bb[1].z = MAX(bb[1].z, pos.z);
            }
            nearPlane       = dist;
            float size      = MAX(bb[1].x - bb[0].x, bb[1].y - bb[0].y);
            Matrix4 proj    = Matrix4::ortho(bb[0].x, bb[0].x + size,
                                             bb[0].y, bb[0].y + size,
                                             -100, 100);

            m_pMatrix[lod]  = Matrix4(Vector3(0.5f), Quaternion(), Vector3(0.5f)) * proj * view;
            // Draw in the depth buffer from position of the light source
            b->setViewProjection(view, proj);
            uint32_t x  = (lod % 2) * m_Resolution;
            uint32_t y  = (lod / 2) * m_Resolution;
            b->setViewport(x, y, m_Resolution, m_Resolution);

            m_pTiles[lod]   = Vector4((float)x / (float)m_DepthMap.width(),
                                      (float)y / (float)m_DepthMap.height(),
                                      (float)m_Resolution / (float)m_DepthMap.width(),
                                      (float)m_Resolution / (float)m_DepthMap.height());

            pipeline.drawComponents(*(pipeline.scene()), ICommandBuffer::SHADOWCAST);
        }
    }
}
