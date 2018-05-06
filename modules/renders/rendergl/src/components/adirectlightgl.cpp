#include "components/adirectlightgl.h"

#include <float.h>

#include "agl.h"
#include "commandbuffer.h"
#include "apipeline.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/camera.h"

#include "resources/material.h"

#define MAX_LODS 4
#define SM_RESOLUTION_DEFAULT 1024

ADirectLightGL::ADirectLightGL() {
    m_pMatrix       = nullptr;
    m_Resolution    = SM_RESOLUTION_DEFAULT;
    m_CSM           = true;
    m_pPlane        = Engine::loadResource<Mesh>(".embedded/plane.fbx");

    m_LODCount      = (m_CSM) ? MAX_LODS : 1;
    m_pMatrix       = new Matrix4[m_LODCount];
    m_pTiles        = new Vector4[m_LODCount];

    m_pMaterial     = Engine::loadResource<Material>(".embedded/VSM.mtl");
    m_pMaterialInstance = m_pMaterial->createInstance();
}

void ADirectLightGL::draw(APipeline &pipeline, uint8_t layer) {
    if(m_pMaterial) {
        ICommandBuffer *b   = pipeline.buffer();

        m_pMaterialInstance->setTexture("layer0",       pipeline.pipelineTexture("layer0"));
        m_pMaterialInstance->setTexture("layer1",       pipeline.pipelineTexture("layer1"));
        m_pMaterialInstance->setTexture("layer2",       pipeline.pipelineTexture("layer2"));
        m_pMaterialInstance->setTexture("layer3",       pipeline.pipelineTexture("layer3"));
        m_pMaterialInstance->setTexture("depthMap",     pipeline.pipelineTexture("depthMap"));
        m_pMaterialInstance->setTexture("shadowMap",    pipeline.pipelineTexture("shadowMap"));

        m_pMaterialInstance->setVector4("light.color",      &m_Color);
        m_pMaterialInstance->setFloat("light.brightness",   &m_Brightness);
        m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);

        if(m_Shadows) {
            m_pMaterialInstance->setFloat("light.bias",     &m_Bias);
            m_pMaterialInstance->setVector4("light.lod",    &m_NormalizedDistance);

            m_pMaterialInstance->setMatrix4("light.matrix", m_pMatrix, m_LODCount);
            m_pMaterialInstance->setVector4("light.tiles",  m_pTiles,  m_LODCount);
        }

        b->setViewProjection(Matrix4(), Matrix4::ortho( 0.5f,-0.5f,-0.5f, 0.5f, 0.0f, 1.0f));
        b->drawMesh(Matrix4(), m_pPlane, 0, layer, m_pMaterialInstance);
    }
}

void ADirectLightGL::shadowsUpdate(APipeline &pipeline) {
    Camera *camera  = pipeline.activeCamera();
    if(isEnable() && m_Shadows && camera) {
        ICommandBuffer *b   = pipeline.buffer();

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

            m_pTiles[lod]   = Vector4((float)x / 2048.0f,
                                      (float)y / 2048.0f,
                                      (float)m_Resolution / 2048.0f,
                                      (float)m_Resolution / 2048.0f);

            pipeline.drawComponents(*(pipeline.scene()), ICommandBuffer::SHADOWCAST);
        }
    }
}
