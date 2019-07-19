#include "components/spotlight.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

class SpotLightPrivate {
public:
    SpotLightPrivate() :
            m_Angle(45.0f),
            m_Near(0.1f) {
    }

    Vector3 m_Position;
    Vector3 m_Direction;

    Vector4 m_Tiles;

    Matrix4 m_Matrix;

    float m_Angle;
    float m_Near;
};
/*!
    \class SpotLight
    \brief A Spot Light emits light from a single point in a cone shape.
    \inmodule Engine

    To determine the emitter position and emit direction SpotLight uses Transform component of the own Actor.
*/

SpotLight::SpotLight() :
        p_ptr(new SpotLightPrivate) {

    setAngle(45.0f);

    setShape(Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001"));

    Material *material = Engine::loadResource<Material>(".embedded/SpotLight.mtl");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        instance->setVector3("light.position",   &p_ptr->m_Position);
        instance->setVector3("light.direction",  &p_ptr->m_Direction);

        instance->setMatrix4("light.matrix",     &p_ptr->m_Matrix);
        instance->setVector4("light.tiles",      &p_ptr->m_Tiles);

        setMaterial(instance);
    }
}

SpotLight::~SpotLight() {
    delete p_ptr;
}
/*!
    \internal
*/
void SpotLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & ICommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();
        p_ptr->m_Position = actor()->transform()->worldPosition();

        p_ptr->m_Direction = q * Vector3(0.0f, 0.0f, 1.0f);

        Vector4 p = params();

        Matrix4 t(p_ptr->m_Position - p_ptr->m_Direction * distance() * 0.5f,
                  q, Vector3(p.y * 1.5f, p.y * 1.5f, p.y)); // (1.0f - p.z)

        buffer.drawMesh(t, mesh, layer, instance);
    }
}
/*!
    \internal
*/
void SpotLight::shadowsUpdate(const Camera &camera, ICommandBuffer &buffer, ObjectList &components) {
    A_UNUSED(camera)

    Transform *t = actor()->transform();
    Vector3 pos = t->worldPosition();
    Quaternion rot = t->worldRotation();
    Matrix4 view = Matrix4(rot.toMatrix()).inverse();

    Matrix4 scale;
    scale[0]  = 0.5f;
    scale[5]  = 0.5f;
    scale[10] = 0.5f;

    scale[12] = 0.5f;
    scale[13] = 0.5f;
    scale[14] = 0.5f;

    float zFar = params().y;
    Matrix4 crop = Matrix4::perspective(p_ptr->m_Angle, 1.0f, p_ptr->m_Near, zFar);

    p_ptr->m_Matrix = scale * crop * view;

    int32_t x  = (0 % 2) * SM_RESOLUTION_DEFAULT;
    int32_t y  = (0 / 2) * SM_RESOLUTION_DEFAULT;
    int32_t w  = SM_RESOLUTION_DEFAULT;
    int32_t h  = SM_RESOLUTION_DEFAULT;

    p_ptr->m_Tiles = Vector4(static_cast<float>(x) / SM_RESOLUTION,
                             static_cast<float>(y) / SM_RESOLUTION,
                             static_cast<float>(w) / SM_RESOLUTION,
                             static_cast<float>(h) / SM_RESOLUTION);

    buffer.setViewProjection(view, crop);
    buffer.setViewport(x, y, w, h);

    ObjectList filter = Camera::frustumCulling(components,
                                               Camera::frustumCorners(false, p_ptr->m_Angle, 1.0f, pos, rot, p_ptr->m_Near, zFar));
    // Draw in the depth buffer from position of the light source
    for(auto it : filter) {
        static_cast<Renderable *>(it)->draw(buffer, ICommandBuffer::SHADOWCAST);
    }
}
/*!
    Returns the attenuation distance of the light cone.
*/
float SpotLight::distance() const {
    return params().y;
}
/*!
    Changes the attenuation \a distance of the light cone.
*/
void SpotLight::setDistance(float distance) {
    Vector4 p = params();
    p.y = distance;
    setParams(p);
}
/*!
    Returns the angle of the light cone in degrees.
*/
float SpotLight::angle() const {
    return p_ptr->m_Angle;
}
/*!
    Changes the \a angle of the light cone in degrees.
*/
void SpotLight::setAngle(float angle) {
    p_ptr->m_Angle = angle;
    Vector4 p = params();
    p.z = cos(DEG2RAD * p_ptr->m_Angle);
    setParams(p);
}

#ifdef NEXT_SHARED
#include "handles.h"

bool SpotLight::drawHandles() {
    Vector3 pos = actor()->transform()->position();

    Handles::s_Color = Handles::s_Second = color();
    bool result = Handles::drawBillboard(pos, Vector2(0.1), Engine::loadResource<Texture>(".embedded/spotlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
