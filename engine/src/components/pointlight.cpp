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

    instance->setVector3("light.position",   &p_ptr->m_Position);

    instance->setMatrix4("light.matrix",     p_ptr->m_Matrix, 6);
    instance->setVector4("light.tiles",      p_ptr->m_Tiles, 6);

    setMaterial(instance);
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

        Vector4 p = params();

        Matrix4 t(Vector3(p_ptr->m_Position),
                  Quaternion(),
                  Vector3(p.y * 2.0f, p.y * 2.0f, p.y * 2.0f));

        buffer.setGlobalTexture(SHADOW_MAP, p_ptr->m_pTarget);

        buffer.drawMesh(t, mesh, layer, instance);
    }
}
/*!
    \internal
*/
void PointLight::shadowsUpdate(const Camera &camera, Pipeline *pipeline, ObjectList &components) {
    A_UNUSED(camera)

    if(!castShadows()) {
        p_ptr->m_pTarget = nullptr;
        return;
    }

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

    float zFar = params().y;
    Matrix4 crop = Matrix4::perspective(90.0f, 1.0f, p_ptr->m_Near, zFar);

    for(int32_t i = 0; i < 6; i++) {
        Matrix4 view = (t->worldTransform() * Matrix4(rot[i].toMatrix())).inverse();

        p_ptr->m_Matrix[i] = scale * crop * view;
        p_ptr->m_Tiles[i] = Vector4(static_cast<float>(x[i]) / pageWidth,
                                    static_cast<float>(y[i]) / pageHeight,
                                    static_cast<float>(w[i]) / pageWidth,
                                    static_cast<float>(h[i]) / pageHeight);

        ICommandBuffer *buffer = pipeline->buffer();
        buffer->setRenderTarget(TargetBuffer(), p_ptr->m_pTarget);
        buffer->enableScissor(x[i], y[i], w[i], h[i]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(view, crop);
        buffer->setViewport(x[i], y[i], w[i], h[i]);

        ObjectList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(false, 90.0f, 1.0f, pos, rot[i], p_ptr->m_Near, zFar));
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, ICommandBuffer::SHADOWCAST);
        }
    }
}
/*!
    Returns the attenuation radius of the light.
*/
float PointLight::radius() const {
    return params().y;
}
/*!
    Changes the attenuation \a radius of the light.
*/
void PointLight::setRadius(float radius) {
    Vector4 p = params();
    p.y = radius;
    setParams(p);

    p_ptr->m_Box = AABBox(Vector3(), Vector3(radius * 2));
}

AABBox PointLight::bound() const {
    return p_ptr->m_Box * actor()->transform()->worldTransform();
}

#ifdef NEXT_SHARED
#include "handles.h"

bool PointLight::drawHandles(bool selected) {
    A_UNUSED(selected);
    Vector3 pos = actor()->transform()->position();

    Handles::s_Color = Handles::s_Second = color();
    bool result = Handles::drawBillboard(pos, Vector2(0.5f), Engine::loadResource<Texture>(".embedded/pointlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
