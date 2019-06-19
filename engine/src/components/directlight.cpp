#include "components/directlight.h"

#include "components/actor.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"

#define MAX_LODS 4

class DirectLightPrivate {
public:
    DirectLightPrivate() {
        m_pMatrix       = new Matrix4[MAX_LODS];
        m_pTiles        = new Vector4[MAX_LODS];
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

DirectLight::DirectLight() :
        p_ptr(new DirectLightPrivate) {
    m_pShape = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    Material *material  = Engine::loadResource<Material>(".embedded/DirectLight.mtl");
    m_pMaterialInstance = material->createInstance();

    m_pMaterialInstance->setVector4("light.color",      &m_Color);
    m_pMaterialInstance->setVector4("light.lod",        &p_ptr->m_NormalizedDistance);
    m_pMaterialInstance->setVector4("light.params",     &m_Params);

    m_pMaterialInstance->setVector3("light.direction",  &p_ptr->m_Direction);

    m_pMaterialInstance->setFloat("light.shadows",      &m_Shadows);
    m_pMaterialInstance->setFloat("light.bias",         &m_Bias);

    m_pMaterialInstance->setMatrix4("light.matrix",     p_ptr->m_pMatrix, MAX_LODS);
    m_pMaterialInstance->setVector4("light.tiles",      p_ptr->m_pTiles,  MAX_LODS);
}

DirectLight::~DirectLight() {
    delete p_ptr;
}

void DirectLight::draw(ICommandBuffer &buffer, uint32_t layer) {
    if(m_pShape && m_pMaterialInstance && (layer & ICommandBuffer::LIGHT)) {
        Matrix4 m = actor()->transform()->worldTransform();

        p_ptr->m_Direction = m.rotation() * Vector3(0.0f, 0.0f, 1.0f);

        buffer.setScreenProjection();
        buffer.drawMesh(Matrix4(), m_pShape, 0, layer, m_pMaterialInstance);
        buffer.resetViewProjection();
    }
}

Vector4 &DirectLight::normalizedDistance() {
    return p_ptr->m_NormalizedDistance;
}

Vector4 *DirectLight::tiles() {
    return p_ptr->m_pTiles;
}

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
    bool result = Handles::drawBillboard(pos, Vector2(1.0), Engine::loadResource<Texture>(".embedded/directlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
