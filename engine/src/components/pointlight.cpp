#include "components/pointlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/pipeline.h"
#include "resources/rendertexture.h"

static Quaternion rot[6] = {Quaternion(Vector3(0, 1, 0),-90),
                            Quaternion(Vector3(0, 1, 0), 90),
                            Quaternion(Vector3(1, 0, 0), 90),
                            Quaternion(Vector3(1, 0, 0),-90),
                            Quaternion(Vector3(0, 1, 0),180),
                            Quaternion()};

class PointLightPrivate {
public:
    PointLightPrivate() :
            m_Near(0.1f) {

    }

    Vector3 m_Position;

    Vector3 m_Direction;

    Vector4 m_Tiles[6];

    Matrix4 m_Matrix[6];

    float m_Near;

    RenderTexture *m_pTarget;

    AABBox m_Box;
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

    Material *material = Engine::loadResource<Material>(".embedded/PointLight.mtl");
    MaterialInstance *instance = material->createInstance();

    instance->setVector3("light.position", &p_ptr->m_Position);
    instance->setVector3("light.direction", &p_ptr->m_Direction);

    instance->setMatrix4("light.matrix", p_ptr->m_Matrix, 6);
    instance->setVector4("light.tiles",  p_ptr->m_Tiles, 6);

    setMaterial(instance);

    Vector4 p = params();
    p.y = 0.1f;
    setParams(p);
}

PointLight::~PointLight() {
    delete p_ptr;
}
/*!
    \internal
*/
void PointLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & ICommandBuffer::LIGHT)) {

        Matrix4 m = actor()->transform()->worldTransform();
        p_ptr->m_Position = Vector3(m[12], m[13], m[14]);

        float r = radius();

        Matrix4 t(p_ptr->m_Position,
                  Quaternion(),
                  Vector3(r * 2.0f, r * 2.0f, r * 2.0f));

        p_ptr->m_Direction = m.rotation() * Vector3(0.0f, 1.0f, 0.0f);

        buffer.setGlobalTexture(SHADOW_MAP, p_ptr->m_pTarget);

        buffer.drawMesh(t, mesh, layer, instance);
    }
}
/*!
    \internal
*/
void PointLight::shadowsUpdate(const Camera &camera, Pipeline *pipeline, ObjectList &components) {
    A_UNUSED(camera);

    if(!castShadows()) {
        p_ptr->m_pTarget = nullptr;
        return;
    }

    ICommandBuffer *buffer = pipeline->buffer();

    Transform *t = actor()->transform();
    Vector3 pos = t->worldPosition();

    Matrix4 scale;
    scale[0]  = 0.5f;
    scale[5]  = 0.5f;
    scale[10] = 0.5f;

    scale[12] = 0.5f;
    scale[13] = 0.5f;
    scale[14] = 0.5f;

    int32_t x[6], y[6], w[6], h[6];
    p_ptr->m_pTarget = pipeline->requestShadowTiles(uuid(), 1, x, y, w, h, 6);

    int32_t pageWidth, pageHeight;
    Pipeline::shadowPageSize(pageWidth, pageHeight);

    float zFar = radius();
    Matrix4 crop = Matrix4::perspective(90.0f, 1.0f, p_ptr->m_Near, zFar);

    Matrix4 wt = t->worldTransform();

    Matrix4 wp;
    wp.translate(Vector3(wt[12], wt[13], wt[14]));

    for(int32_t i = 0; i < 6; i++) {
        Matrix4 mat = (wp * Matrix4(rot[i].toMatrix())).inverse();
        p_ptr->m_Matrix[i] = scale * crop * mat;

        p_ptr->m_Tiles[i] = Vector4(static_cast<float>(x[i]) / pageWidth,
                                    static_cast<float>(y[i]) / pageHeight,
                                    static_cast<float>(w[i]) / pageWidth,
                                    static_cast<float>(h[i]) / pageHeight);

        buffer->setRenderTarget(TargetBuffer(), p_ptr->m_pTarget);
        buffer->enableScissor(x[i], y[i], w[i], h[i]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(mat, crop);
        buffer->setViewport(x[i], y[i], w[i], h[i]);

        ObjectList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(false, 90.0f, 1.0f, pos, rot[i], p_ptr->m_Near, zFar));
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, ICommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }
}
/*!
    Returns the attenuation radius of the light.
*/
float PointLight::radius() const {
    return params().w;
}
/*!
    Changes the attenuation \a radius of the light.
*/
void PointLight::setRadius(float radius) {
    Vector4 p = params();
    p.w = radius;
    setParams(p);

    p_ptr->m_Box = AABBox(Vector3(), Vector3(radius * 2));
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
    return p_ptr->m_Box * actor()->transform()->worldTransform();
}

#ifdef NEXT_SHARED
#include "handles.h"

bool PointLight::drawHandles(bool selected) {
    A_UNUSED(selected);
    Transform *t = actor()->transform();

    if(selected) {
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
