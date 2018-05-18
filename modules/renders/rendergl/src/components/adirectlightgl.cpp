#include "components/adirectlightgl.h"

#include <float.h>

#include "agl.h"
#include "commandbuffer.h"
#include "apipeline.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/camera.h"

#define SM_RESOLUTION_DEFAULT 1024
#define SM_RESOLUTION 2048

#define MAX_LODS 4

void ADirectLightGL::shadowsUpdate(APipeline &pipeline) {
    Camera *camera  = pipeline.activeCamera();
    if(isEnable() && m_Shadows && camera) {
        ICommandBuffer *b   = pipeline.buffer();

        Vector4 distance;
        Matrix4 mv, p;
        camera->matrices(mv, p);
        {
            float split     = 0.9f;
            float nearPlane = camera->nearPlane();
            float farPlane  = camera->farPlane();
            for(int i = 1; i <= MAX_LODS; i++) {
                float f = (float)i / (float)MAX_LODS;
                float l = nearPlane * pow(farPlane / nearPlane, f);
                float u = nearPlane + (farPlane - nearPlane) * f;
                float v = MIX(u, l, split);
                distance[i - 1] = v;
                Vector4 depth   = p * Vector4(0.0f, 0.0f, -v, 1.0f);
                m_NormalizedDistance[i - 1] = (depth.z / depth.w);
            }
        }

        float nearPlane = camera->nearPlane();
        Matrix4 view    = Matrix4(actor().rotation().toMatrix()).inverse();
        Matrix4 inv     = mv.inverse();
        for(uint32_t lod = 0; lod < MAX_LODS; lod++) {
            float dist  = distance[lod];
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
            Matrix4 proj    = Matrix4::ortho(bb[0].x, bb[1].x,
                                             bb[0].y, bb[1].y,
                                             -100, 100);

            m_pMatrix[lod]  = Matrix4(Vector3(0.5f), Quaternion(), Vector3(0.5f)) * proj * view;
            // Draw in the depth buffer from position of the light source
            b->setViewProjection(view, proj);
            uint32_t x  = (lod % 2) * SM_RESOLUTION_DEFAULT;
            uint32_t y  = (lod / 2) * SM_RESOLUTION_DEFAULT;
            b->setViewport(x, y, SM_RESOLUTION_DEFAULT, SM_RESOLUTION_DEFAULT);

            m_pTiles[lod]   = Vector4((float)x / SM_RESOLUTION,
                                      (float)y / SM_RESOLUTION,
                                      (float)SM_RESOLUTION_DEFAULT / SM_RESOLUTION,
                                      (float)SM_RESOLUTION_DEFAULT / SM_RESOLUTION);

            pipeline.drawComponents(*(pipeline.scene()), ICommandBuffer::SHADOWCAST);
        }
    }
}
