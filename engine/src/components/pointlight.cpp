#include "components/pointlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/rendertarget.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"

#define SIDES 6

namespace {
const char *uni_position  = "uni.position";
const char *uni_direction = "uni.direction";
const char *uni_matrix    = "uni.matrix";
const char *uni_tiles     = "uni.tiles";
};

static Quaternion rot[SIDES] = {Quaternion(Vector3(0, 1, 0),-90),
                                Quaternion(Vector3(0, 1, 0), 90),
                                Quaternion(Vector3(1, 0, 0), 90),
                                Quaternion(Vector3(1, 0, 0),-90),
                                Quaternion(Vector3(0, 1, 0),180),
                                Quaternion()};

class PointLightPrivate {
public:
    PointLightPrivate() :
            m_near(0.1f),
            m_shadowMap(nullptr) {

    }

    Vector3 m_position;

    Vector3 m_direction;

    Vector4 m_tiles[SIDES];

    Matrix4 m_matrix[SIDES];

    float m_near;

    RenderTarget *m_shadowMap;

    AABBox m_box;
};

/*!
    \class PointLight
    \brief Point Lights works much like a real-world light bulb, emitting light in all directions.
    \inmodule Engine

    To determine the emiter position PointLight uses Transform component of the own Actor.
*/

PointLight::PointLight() :
        p_ptr(new PointLightPrivate) {
    setShape(Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001"));

    Material *material = Engine::loadResource<Material>(".embedded/PointLight.shader");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        setMaterial(instance);
    }

    Vector4 p = params();
    p.y = 0.1f;
    setParams(p);
}

PointLight::~PointLight() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void PointLight::draw(CommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & CommandBuffer::LIGHT)) {

        Matrix4 m = actor()->transform()->worldTransform();
        p_ptr->m_position = Vector3(m[12], m[13], m[14]);

        float r = attenuationRadius();
        Matrix4 t(p_ptr->m_position,
                  Quaternion(),
                  Vector3(r * 2.0f, r * 2.0f, r * 2.0f));

        p_ptr->m_direction = m.rotation() * Vector3(0.0f, 1.0f, 0.0f);

        instance->setVector3(uni_position, &p_ptr->m_position);
        instance->setVector3(uni_direction, &p_ptr->m_direction);

        buffer.setGlobalTexture(SHADOW_MAP, (p_ptr->m_shadowMap) ? p_ptr->m_shadowMap->depthAttachment() : nullptr);

        buffer.drawMesh(t, mesh, 0, layer, instance);
    }
}
/*!
    \internal
*/
void PointLight::shadowsUpdate(const Camera &camera, PipelineContext *context, RenderList &components) {
    A_UNUSED(camera);

    if(!castShadows()) {
        p_ptr->m_shadowMap = nullptr;
        return;
    }

    CommandBuffer *buffer = context->buffer();

    Transform *t = actor()->transform();
    Vector3 pos = t->worldPosition();

    Matrix4 scale;
    scale[0]  = 0.5f;
    scale[5]  = 0.5f;
    scale[10] = 0.5f;

    scale[12] = 0.5f;
    scale[13] = 0.5f;
    scale[14] = 0.5f;

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    p_ptr->m_shadowMap = context->requestShadowTiles(uuid(), 1, x, y, w, h, SIDES);

    int32_t pageWidth, pageHeight;
    context->shadowPageSize(pageWidth, pageHeight);

    float zFar = attenuationRadius();
    Matrix4 crop = Matrix4::perspective(90.0f, 1.0f, p_ptr->m_near, zFar);

    Matrix4 wt = t->worldTransform();

    Matrix4 wp;
    wp.translate(Vector3(wt[12], wt[13], wt[14]));

    for(int32_t i = 0; i < 6; i++) {
        Matrix4 mat = (wp * Matrix4(rot[i].toMatrix())).inverse();
        p_ptr->m_matrix[i] = scale * crop * mat;

        p_ptr->m_tiles[i] = Vector4(static_cast<float>(x[i]) / pageWidth,
                                    static_cast<float>(y[i]) / pageHeight,
                                    static_cast<float>(w[i]) / pageWidth,
                                    static_cast<float>(h[i]) / pageHeight);

        buffer->setRenderTarget(p_ptr->m_shadowMap);
        buffer->enableScissor(x[i], y[i], w[i], h[i]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(mat, crop);
        buffer->setViewport(x[i], y[i], w[i], h[i]);

        RenderList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(false, 90.0f, 1.0f, pos, rot[i], p_ptr->m_near, zFar));
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }
    auto instance = material();
    if(instance) {
        instance->setMatrix4(uni_matrix, p_ptr->m_matrix, SIDES);
        instance->setVector4(uni_tiles,  p_ptr->m_tiles, SIDES);
    }
}
/*!
    Returns the attenuation radius of the light.
*/
float PointLight::attenuationRadius() const {
    return params().w;
}
/*!
    Changes the attenuation \a radius of the light.
*/
void PointLight::setAttenuationRadius(float radius) {
    Vector4 p = params();
    p.w = radius;
    setParams(p);

    p_ptr->m_box = AABBox(Vector3(), Vector3(radius * 2));
}
/*!
    Returns the source radius of the light.
*/
float PointLight::sourceRadius() const {
    return params().y;
}
/*!
    Changes the source \a radius of the light.
*/
void PointLight::setSourceRadius(float radius) {
    Vector4 p = params();
    p.y = radius;
    setParams(p);
}
/*!
    Returns the source length of the light.
*/
float PointLight::sourceLength() const {
    return params().z;
}
/*!
    Changes the source \a length of the light.
*/
void PointLight::setSourceLength(float length) {
    Vector4 p = params();
    p.z = length;
    setParams(p);
}

/*!
    \internal
*/
AABBox PointLight::bound() const {
    AABBox result = p_ptr->m_box;
    result.center += actor()->transform()->worldPosition();
    return result;
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool PointLight::drawHandles(ObjectList &selected) {
    Transform *t = actor()->transform();
    if(isSelected(selected)) {
        Vector4 p = params();
        Handles::s_Color = Vector4(0.5f, 1.0f, 1.0f, 1.0f);
        Handles::drawSphere(t->worldPosition(), t->worldRotation(), p.w);

        Handles::s_Color = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
        Handles::drawCapsule(t->worldPosition(), t->worldRotation(), p.y, p.z + p.y * 2.0f);
    }
    Handles::s_Color = Handles::s_Second = color();
    bool result = Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/pointlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
