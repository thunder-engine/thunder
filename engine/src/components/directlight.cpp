#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/pipeline.h"
#include "resources/rendertarget.h"

#include "systems/rendersystem.h"

#include <float.h>

#define MAX_LODS 4

#define SPLIT_WEIGHT 0.95f // 0.75f

class DirectLightPrivate {
public:
    Matrix4 m_matrix[MAX_LODS];
    Vector4 m_tiles[MAX_LODS];

    Vector4 m_normalizedDistance;

    Vector3 m_direction;

    RenderTarget *m_shadowMap;
};
/*!
    \class DirectLight
    \brief The Directional Light simulates light that is being emitted from a source that is infinitely far away.
    \inmodule Engine

    To determine the emit direction DirectLight uses Transform component of the own Actor.
*/

DirectLight::DirectLight() :
        p_ptr(new DirectLightPrivate) {
    setShape(Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001"));

    Material *material  = Engine::loadResource<Material>(".embedded/DirectLight.mtl");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        instance->setVector4("light.lod",       &p_ptr->m_normalizedDistance);
        instance->setVector3("light.direction", &p_ptr->m_direction);

        instance->setMatrix4("light.matrix", p_ptr->m_matrix, MAX_LODS);
        instance->setVector4("light.tiles",  p_ptr->m_tiles,  MAX_LODS);

        setMaterial(instance);
    }
}

DirectLight::~DirectLight() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void DirectLight::draw(CommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & CommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();

        p_ptr->m_direction = q * Vector3(0.0f, 0.0f, 1.0f);

        buffer.setGlobalTexture(SHADOW_MAP, (p_ptr->m_shadowMap) ? p_ptr->m_shadowMap->depthAttachment() : nullptr);

        buffer.setScreenProjection();
        buffer.drawMesh(Matrix4(), mesh, 0, layer, instance);
        buffer.resetViewProjection();
    }
}
/*!
    \internal
*/
void DirectLight::shadowsUpdate(const Camera &camera, Pipeline *pipeline, RenderList &components) {
    if(!castShadows()) {
        p_ptr->m_shadowMap = nullptr;
        return;
    }

    Vector4 distance;

    float nearPlane = camera.nearPlane();

    CommandBuffer *buffer = pipeline->buffer();
    Matrix4 p = buffer->projection();

    {
        float split    = SPLIT_WEIGHT;
        float farPlane = camera.farPlane();
        float ratio = farPlane / nearPlane;

        for(int i = 0; i < MAX_LODS; i++) {
            float f = (i + 1) / static_cast<float>(MAX_LODS);
            float l = nearPlane * powf(ratio, f);
            float u = nearPlane + (farPlane - nearPlane) * f;
            float val = MIX(u, l, split);
            distance[i] = val;
            Vector4 depth = p * Vector4(0.0f, 0.0f, -val * 2.0f - 1.0f, 1.0f);
            p_ptr->m_normalizedDistance[i] = depth.z / depth.w;
        }
    }

    Quaternion q = actor()->transform()->worldRotation();
    Matrix4 rot = Matrix4(q.toMatrix()).inverse();

    Matrix4 scale;
    scale[0]  = 0.5f;
    scale[5]  = 0.5f;
    scale[10] = 0.5f;

    scale[12] = 0.5f;
    scale[13] = 0.5f;
    scale[14] = 0.5f;

    Transform *t = camera.actor()->transform();
    bool orthographic = camera.orthographic();
    float sigma = (camera.orthographic()) ? camera.orthoSize() : camera.fov();
    float ratio = camera.ratio();
    Vector3 wPosition = t->worldPosition();
    Quaternion wRotation = t->worldRotation();

    int32_t x[MAX_LODS], y[MAX_LODS], w[MAX_LODS], h[MAX_LODS];
    p_ptr->m_shadowMap = pipeline->requestShadowTiles(uuid(), 0, x, y, w, h, MAX_LODS);

    int32_t pageWidth, pageHeight;
    RenderSystem::atlasPageSize(pageWidth, pageHeight);

    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist = distance[lod];
        const array<Vector3, 8> &points = Camera::frustumCorners(orthographic, sigma, ratio, wPosition, wRotation, nearPlane, dist);
        nearPlane = dist;

        AABBox box;
        box.setBox(&(points.at(0)), 8);

        Vector3 min, max;
        box.box(min, max);

        Vector3 rotPoints[8]  = {
            rot * Vector3(min.x, min.y, min.z),
            rot * Vector3(min.x, min.y, max.z),
            rot * Vector3(max.x, min.y, max.z),
            rot * Vector3(max.x, min.y, min.z),

            rot * Vector3(min.x, max.y, min.z),
            rot * Vector3(min.x, max.y, max.z),
            rot * Vector3(max.x, max.y, max.z),
            rot * Vector3(max.x, max.y, min.z)
        };

        min.x = FLT_MAX;
        max.x =-FLT_MAX;

        min.y = FLT_MAX;
        max.y =-FLT_MAX;

        for(uint32_t i = 0; i < 8; i++) {
            min.x = MIN(min.x, rotPoints[i].x);
            max.x = MAX(max.x, rotPoints[i].x);

            min.y = MIN(min.y, rotPoints[i].y);
            max.y = MAX(max.y, rotPoints[i].y);
        }

        min.z =-100.0f; /// \todo Must be replaced by the calculations
        max.z = 100.0f;

        Matrix4 crop = Matrix4::ortho(min.x, max.x, min.y, max.y, min.z, max.z);

        p_ptr->m_matrix[lod] = scale * crop * rot;

        p_ptr->m_tiles[lod] = Vector4(static_cast<float>(x[lod]) / pageWidth,
                                       static_cast<float>(y[lod]) / pageHeight,
                                       static_cast<float>(w[lod]) / pageWidth,
                                       static_cast<float>(h[lod]) / pageHeight);

        buffer->setRenderTarget(p_ptr->m_shadowMap);
        buffer->enableScissor(x[lod], y[lod], w[lod], h[lod]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(rot, crop);
        buffer->setViewport(x[lod], y[lod], w[lod], h[lod]);

        Vector3 size = max - min;
        Vector3 pos(min + size * 0.5f);

        RenderList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(true, max.y - min.y, 1.0f, pos, q, min.z, max.z));

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
    }
}
/*!
    \internal
*/
AABBox DirectLight::bound() const {
    return AABBox(0.0f, -1.0f);
}

#ifdef NEXT_SHARED
#include "handles.h"

bool DirectLight::drawHandles(ObjectList &selected) {
    A_UNUSED(selected);
    Transform *t = actor()->transform();

    Matrix4 z(Vector3(), Quaternion(Vector3(1, 0, 0),-90), Vector3(1.0));
    Handles::s_Color = Handles::s_Second = color();
    Handles::drawArrow(Matrix4(t->worldPosition(), t->worldRotation(), Vector3(0.25f)) * z);
    bool result = Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/directlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
