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

PointLight::PointLight() :
        p_ptr(new PointLightPrivate) {
    m_pShape = Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001");

    Material *material  = Engine::loadResource<Material>(".embedded/PointLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector3("light.position",   &p_ptr->m_Position);
    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setVector4("light.params",     &m_Params);

    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);
}

PointLight::~PointLight() {
    delete p_ptr;
}

void PointLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {

        Matrix4 m = actor()->transform()->worldTransform();
        p_ptr->m_Position = Vector3(m[12], m[13], m[14]);

        Matrix4 t(Vector3(p_ptr->m_Position),
                  Quaternion(),
                  Vector3(m_Params.y * 2.0f, m_Params.y * 2.0f, m_Params.y * 2.0f));

        buffer.drawMesh(t, m_pShape, 0, layer, m_pMaterialInstance);
    }
}

float PointLight::radius() const {
    return m_Params.y;
}

void PointLight::setRadius(float value) {
    m_Params.y = value;
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
