#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "pipelinecontext.h"
#include "gizmos.h"

#define MAX_LODS 4
#define SPLIT_WEIGHT 0.95f // 0.75f

namespace {
    const char *uniMatrix("matrix");
    const char *uniBias("bias");
    const char *uniPlaneDistance("planeDistance");
}

/*!
    \class DirectLight
    \brief The Directional Light simulates light that is being emitted from a source that is infinitely far away.
    \inmodule Components

    To determine the emit direction DirectLight uses Transform component of the own Actor.
*/

DirectLight::DirectLight() :
        m_camera(nullptr) {

    Material *material = Engine::loadResource<Material>(".embedded/DirectLight.shader");
    if(material) {
        setMaterial(material->createInstance());
    }
}
/*!
    Sets a camera associated with current light source.
    This camera will be used to calculate light location because this type light of source is always following the viewer.
*/
Camera *DirectLight::camera() const {
    return m_camera;
}
/*!
    Sets a \a camera associated with current light source.
*/
void DirectLight::setCamera(Camera *camera) {
    if(m_camera != camera) {
        m_camera = camera;
        m_dirty = true;
    }
}
/*!
    \internal
*/
void DirectLight::cleanDirty() {
    Camera *camera = m_camera;
    if(camera == nullptr) {
        camera = Camera::current();
    }

    float nearPlane = camera->nearPlane();

    Matrix4 p(camera->projectionMatrix());

    float split = SPLIT_WEIGHT;
    float farPlane = camera->farPlane();
    float ratio = farPlane / nearPlane;

    Vector4 distance;
    Vector4 planeDistance;
    for(int i = 0; i < MAX_LODS; i++) {
        float f = (i + 1) / static_cast<float>(MAX_LODS);
        float l = nearPlane * powf(ratio, f);
        float u = nearPlane + (farPlane - nearPlane) * f;
        float val = MIX(u, l, split);
        distance[i] = val;
        Vector4 depth = p * Vector4(0.0f, 0.0f, -val * 2.0f - 1.0f, 1.0f);
        planeDistance[i] = depth.z / depth.w;
    }

    Transform *lightTransform = transform();
    Quaternion lightRot(lightTransform->worldQuaternion());
    Matrix4 rot(Matrix4(lightRot.toMatrix()).inverse());

    m_hash = lightTransform->hash();

    Transform *cameraTransform = camera->transform();
    Vector3 cameraPos(cameraTransform->worldPosition());
    Quaternion cameraRot(cameraTransform->worldQuaternion());

    Mathf::hashCombine(m_hash, cameraTransform->hash());

    bool orthographic = camera->orthographic();
    float sigma = (orthographic) ? camera->orthoSize() : camera->fov();
    ratio = camera->ratio();

    m_viewFrustum.resize(MAX_LODS);
    m_cropMatrix.resize(MAX_LODS);
    Matrix4 matrix[MAX_LODS];

    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist = distance[lod];
        auto points = Camera::frustumCorners(orthographic, sigma, ratio, cameraPos, cameraRot, nearPlane, dist);

        nearPlane = dist;

        AABBox box;
        box.setBox(points.data(), 8);
        box *= lightRot;

        m_viewFrustum[lod] = Camera::frustum(true, box.extent.y * 2.0f, 1.0f, box.center, lightRot, -1000.0f, 1000.0f);

        Matrix4 m;
        m.translate(-box.center - lightRot * Vector3(0.0f, 0.0f, box.radius));
        Matrix4 view(rot * m);
        Matrix4 crop(Matrix4::ortho(-box.extent.x, box.extent.x,
                                    -box.extent.y, box.extent.y,
                                    0.0f, box.radius * 2.0f));

        m_cropMatrix[lod] = crop * view;
        matrix[lod] = s_scale * m_cropMatrix[lod];
    }

    if(m_materialInstance) {
        Vector4 bias;

        //const float biasModifier = 0.5f;
        //for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        //    bias[lod] *= 1.0f / (planeDistance[lod] * biasModifier);
        //}

        m_materialInstance->setMatrix4(uniMatrix, matrix, MAX_LODS);
        m_materialInstance->setVector4(uniBias, &bias);
        m_materialInstance->setVector4(uniPlaneDistance, &planeDistance);
    }

    m_dirty = false;
}
/*!
    \internal
*/
int DirectLight::tilesCount() const {
    return MAX_LODS;
}
/*!
    \internal
*/
int DirectLight::lightType() const {
    return BaseLight::DirectLight;
}
/*!
    \internal
*/
void DirectLight::drawGizmos() {
    Transform *t = transform();

    const int steps = 16;

    Vector3Vector vertices;
    vertices.reserve(steps * 2);
    vertices = Mathf::pointsArc(Quaternion(Vector3(1, 0, 0), 90), 0.25f, 0.0f, 360.0f, 16);
    for(int i = 0; i < steps; i++) {
        vertices.push_back(vertices[i] + Vector3(0.0f, 0.0f,-1.0f));
    }

    IndexVector indices;
    indices.resize((steps - 1) * 4);
    for(int i = 0; i < steps - 1; i++) {
        indices[i * 4] = i;
        indices[i * 4 + 1] = i+1;
        indices[i * 4 + 2] = i;
        indices[i * 4 + 3] = i+steps;
    }

    Gizmos::drawLines(vertices, indices, gizmoColor(), t->worldTransform());
    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/directlight.png", color());

    if(m_camera) {

    }
}
