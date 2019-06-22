#include "components/pointlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

class PointLightPrivate {
public:
    Vector3 m_Position;
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

        buffer.drawMesh(t, mesh, layer, instance);
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
}

#ifdef NEXT_SHARED
#include "handles.h"

bool PointLight::drawHandles() {
    Vector3 pos = actor()->transform()->position();

    Handles::s_Color = Handles::s_Second = color();
    bool result = Handles::drawBillboard(pos, Vector2(1.0), Engine::loadResource<Texture>(".embedded/pointlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
