#include "components/spotlight.h"

#include "systems/rendersystem.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "commandbuffer.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/pipeline.h"
#include "resources/rendertexture.h"

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

    RenderTexture *m_pTarget;

    AABBox m_Box;
};
/*!
    \class SpotLight
    \brief A Spot Light emits light from a single point in a cone shape.
    \inmodule Engine

    To determine the emitter position and emit direction SpotLight uses Transform component of the own Actor.
*/

SpotLight::SpotLight() :
        p_ptr(new SpotLightPrivate) {

    setOuterAngle(45.0f);

    setShape(Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001"));

    Material *material = Engine::loadResource<Material>(".embedded/SpotLight.mtl");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        instance->setVector3("light.position",  &p_ptr->m_Position);
        instance->setVector3("light.direction", &p_ptr->m_Direction);

        instance->setMatrix4("light.matrix", &p_ptr->m_Matrix);
        instance->setVector4("light.tiles",  &p_ptr->m_Tiles);

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

        float d = attenuationDistance();

        Matrix4 t(p_ptr->m_Position - p_ptr->m_Direction * d * 0.5f,
                  q, Vector3(d * 1.5f, d * 1.5f, d)); // (1.0f - p.z)

        buffer.setGlobalTexture(SHADOW_MAP, p_ptr->m_pTarget);

        buffer.drawMesh(t, mesh, layer, instance);
    }
}
/*!
    \internal
*/
void SpotLight::shadowsUpdate(const Camera &camera, Pipeline *pipeline, ObjectList &components) {
    A_UNUSED(camera);

    if(!castShadows()) {
        p_ptr->m_pTarget = nullptr;
        return;
    }
    ICommandBuffer *buffer = pipeline->buffer();

    Transform *t = actor()->transform();
    Vector3 pos = t->worldPosition();
    Quaternion q = t->worldRotation();
    Matrix4 rot = t->worldTransform().inverse();

    Matrix4 scale;
    scale[0]  = 0.5f;
    scale[5]  = 0.5f;
    scale[10] = 0.5f;

    scale[12] = 0.5f;
    scale[13] = 0.5f;
    scale[14] = 0.5f;

    float zFar = attenuationDistance();
    Matrix4 crop = Matrix4::perspective(p_ptr->m_Angle * 2.0f, 1.0f, p_ptr->m_Near, zFar);

    int32_t x, y, w, h;
    p_ptr->m_pTarget = pipeline->requestShadowTiles(uuid(), 1, &x, &y, &w, &h, 1);

    int32_t pageWidth, pageHeight;
    RenderSystem::atlasPageSize(pageWidth, pageHeight);

    p_ptr->m_Matrix = scale * crop * rot;
    p_ptr->m_Tiles = Vector4(static_cast<float>(x) / pageWidth,
                             static_cast<float>(y) / pageHeight,
                             static_cast<float>(w) / pageWidth,
                             static_cast<float>(h) / pageHeight);

    buffer->setRenderTarget(TargetBuffer(), p_ptr->m_pTarget);
    buffer->enableScissor(x, y, w, h);
    buffer->clearRenderTarget();
    buffer->disableScissor();

    buffer->setViewProjection(rot, crop);
    buffer->setViewport(x, y, w, h);

    ObjectList filter = Camera::frustumCulling(components,
                                               Camera::frustumCorners(false, p_ptr->m_Angle * 2.0f, 1.0f, pos, q, p_ptr->m_Near, zFar));
    // Draw in the depth buffer from position of the light source
    for(auto it : filter) {
        static_cast<Renderable *>(it)->draw(*buffer, ICommandBuffer::SHADOWCAST);
    }
    buffer->resetViewProjection();
}
/*!
    \internal
*/
AABBox SpotLight::bound() const {
    return p_ptr->m_Box * actor()->transform()->worldTransform();
}

/*!
    Returns the attenuation distance of the light cone.
*/
float SpotLight::attenuationDistance() const {
    return params().y;
}
/*!
    Changes the attenuation \a distance of the light cone.
*/
void SpotLight::setAttenuationDistance(float distance) {
    Vector4 p = params();
    p.y = distance;
    setParams(p);

    p_ptr->m_Box = AABBox(Vector3(0.0f, 0.0f,-0.5f) * distance, Vector3(distance * 1.5f, distance * 1.5f, distance));
}
/*!
    Returns the angle of the light cone in degrees.
*/
float SpotLight::outerAngle() const {
    return p_ptr->m_Angle;
}
/*!
    Changes the \a angle of the light cone in degrees.
*/
void SpotLight::setOuterAngle(float angle) {
    p_ptr->m_Angle = angle;
    Vector4 p = params();
    p.w = cos(DEG2RAD * p_ptr->m_Angle);
    setParams(p);
}

#ifdef NEXT_SHARED
#include "handles.h"

bool SpotLight::drawHandles(ObjectList &selected) {
    A_UNUSED(selected);
    Transform *t = actor()->transform();

    Matrix4 z(Vector3(), Quaternion(Vector3(1, 0, 0),-90), Vector3(1.0));
    Handles::s_Color = Handles::s_Second = color();
    Handles::drawArrow(Matrix4(t->worldPosition(), t->worldRotation(), Vector3(0.25f)) * z);
    bool result = Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/spotlight.png"));
    Handles::s_Color = Handles::s_Second = Handles::s_Normal;

    return result;
}
#endif
