#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

#define MAX_LODS 4

#define SPLIT_WEIGHT 0.95f // 0.75f

class DirectLightPrivate {
public:
    DirectLightPrivate() {
        m_pMatrix = new Matrix4[MAX_LODS];
        m_pTiles  = new Vector4[MAX_LODS];
    }

    ~DirectLightPrivate() {
        delete m_pMatrix;
        delete m_pTiles;
    }

    Matrix4 *m_pMatrix;
    Vector4 *m_pTiles;

    Vector4  m_NormalizedDistance;

    Vector3  m_Direction;
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

        instance->setVector4("light.lod",        &p_ptr->m_NormalizedDistance);
        instance->setVector3("light.direction",  &p_ptr->m_Direction);

        instance->setMatrix4("light.matrix",     p_ptr->m_pMatrix, MAX_LODS);
        instance->setVector4("light.tiles",      p_ptr->m_pTiles,  MAX_LODS);

        setMaterial(instance);
    }
}

DirectLight::~DirectLight() {
    delete p_ptr;
}
/*!
    \internal
*/
void DirectLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & ICommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();

        p_ptr->m_Direction = q * Vector3(0.0f, 0.0f, 1.0f);

        buffer.setScreenProjection();
        buffer.drawMesh(Matrix4(), mesh, layer, instance);
        buffer.resetViewProjection();
    }
}
/*!
    \internal
*/
void DirectLight::shadowsUpdate(const Camera &camera, ICommandBuffer &buffer, ObjectList &components) {
    Vector4 distance;

    float nearPlane = camera.nearPlane();

    Matrix4 p = camera.projectionMatrix();
    {
        float split     = SPLIT_WEIGHT;
        float farPlane  = camera.farPlane();
        float ratio = farPlane / nearPlane;

        for(int i = 0; i < MAX_LODS; i++) {
            float f = (i + 1) / static_cast<float>(MAX_LODS);
            float l = nearPlane * powf(ratio, f);
            float u = nearPlane + (farPlane - nearPlane) * f;
            float v = MIX(u, l, split);
            distance[i] = v;
            Vector4 depth = p * Vector4(0.0f, 0.0f, -v * 2.0f - 1.0f, 1.0f);
            p_ptr->m_NormalizedDistance[i] = depth.z / depth.w;
        }
    }

    Quaternion rot = actor()->transform()->rotation();
    Matrix4 view = Matrix4(rot.toMatrix()).inverse();

    Matrix4 scale;
    scale[0]  = 0.5f;
    scale[5]  = 0.5f;
    scale[10] = 0.5f;

    scale[12] = 0.5f;
    scale[13] = 0.5f;
    scale[14] = 0.5f;

    Transform *t = camera.actor()->transform();
    bool orthographic = camera.orthographic();
    float sigma = (camera.orthographic()) ? camera.orthoHeight() : camera.fov();
    float ratio = camera.ratio();
    Vector3 wPosition = t->worldPosition();
    Quaternion wRotation = t->worldRotation();

    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist  = distance[lod];
        const array<Vector3, 8> &points = Camera::frustumCorners(orthographic, sigma, ratio, wPosition, wRotation, nearPlane, dist);
        nearPlane = dist;

        AABBox box;
        box.setBox(&(points.at(0)), 8);

        Vector3 min, max;
        box.box(min, max);

        Vector3 rotPoints[8]  = {
            view * Vector3(min.x, min.y, min.z),
            view * Vector3(min.x, min.y, max.z),
            view * Vector3(max.x, min.y, max.z),
            view * Vector3(max.x, min.y, min.z),

            view * Vector3(min.x, max.y, min.z),
            view * Vector3(min.x, max.y, max.z),
            view * Vector3(max.x, max.y, max.z),
            view * Vector3(max.x, max.y, min.z)
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

        p_ptr->m_pMatrix[lod] = scale * crop * view;

        int32_t x  = (lod % 2) * SM_RESOLUTION_DEFAULT;
        int32_t y  = (lod / 2) * SM_RESOLUTION_DEFAULT;
        int32_t w  = SM_RESOLUTION_DEFAULT;
        int32_t h  = SM_RESOLUTION_DEFAULT;

        p_ptr->m_pTiles[lod] = Vector4(static_cast<float>(x) / SM_RESOLUTION,
                                       static_cast<float>(y) / SM_RESOLUTION,
                                       static_cast<float>(w) / SM_RESOLUTION,
                                       static_cast<float>(h) / SM_RESOLUTION);

        buffer.setViewProjection(view, crop);
        buffer.setViewport(x, y, w, h);

        Vector3 size = max - min;
        Vector3 pos(min + size * 0.5f);

        ObjectList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(true, max.y - min.y, 1.0f, pos, rot, min.z, max.z));

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(buffer, ICommandBuffer::SHADOWCAST);
        }
    }
}
/*!
    \internal
*/
Vector4 &DirectLight::normalizedDistance() {
    return p_ptr->m_NormalizedDistance;
}
/*!
    \internal
*/
Vector4 *DirectLight::tiles() {
    return p_ptr->m_pTiles;
}
/*!
    \internal
*/
Matrix4 *DirectLight::matrix() {
    return p_ptr->m_pMatrix;
}

#ifdef NEXT_SHARED
#include "handles.h"

bool DirectLight::drawHandles() {
    Vector3 pos = actor()->transform()->position();

    Matrix4 z(Vector3(), Quaternion(Vector3(1, 0, 0),-90), Vector3(1.0));
    Handles::s_Color = Handles::s_Second = color();
    Handles::drawArrow(Matrix4(pos, actor()->transform()->rotation(), Vector3(0.5f)) * z);
    bool result = Handles::drawBillboard(pos, Vector2(0.1f), Engine::loadResource<Texture>(".embedded/directlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
