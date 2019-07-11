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

    MaterialInstance *instance = material->createInstance();

    instance->setVector4("light.lod",        &p_ptr->m_NormalizedDistance);
    instance->setVector3("light.direction",  &p_ptr->m_Direction);

    instance->setMatrix4("light.matrix",     p_ptr->m_pMatrix, MAX_LODS);
    instance->setVector4("light.tiles",      p_ptr->m_pTiles,  MAX_LODS);

    setMaterial(instance);
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
