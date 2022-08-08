#include "components/spotlight.h"

#include "systems/rendersystem.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"


#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/rendertarget.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"

namespace {
const char *uni_position  = "uni.position";
const char *uni_direction = "uni.direction";
const char *uni_matrix    = "uni.matrix";
const char *uni_tiles     = "uni.tiles";
};

class SpotLightPrivate {
public:
    SpotLightPrivate() :
            m_shadowMap(nullptr),
            m_angle(45.0f),
            m_near(0.1f) {
    }

    AABBox m_box;

    Matrix4 m_matrix;

    Vector3 m_position;
    Vector3 m_direction;

    Vector4 m_tiles;

    RenderTarget *m_shadowMap;

    float m_angle;
    float m_near;
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

    Material *material = Engine::loadResource<Material>(".embedded/SpotLight.shader");
    if(material) {
        MaterialInstance *instance = material->createInstance();

        setMaterial(instance);
    }
}

SpotLight::~SpotLight() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void SpotLight::draw(CommandBuffer &buffer, uint32_t layer) {
    Mesh *mesh = shape();
    MaterialInstance *instance = material();
    if(mesh && instance && (layer & CommandBuffer::LIGHT)) {
        Quaternion q = actor()->transform()->worldRotation();

        p_ptr->m_position = actor()->transform()->worldPosition();
        p_ptr->m_direction = q * Vector3(0.0f, 0.0f, 1.0f);

        instance->setVector3(uni_position,  &p_ptr->m_position);
        instance->setVector3(uni_direction, &p_ptr->m_direction);

        float d = attenuationDistance();

        Matrix4 t(p_ptr->m_position - p_ptr->m_direction * d * 0.5f,
                  q, Vector3(d * 1.5f, d * 1.5f, d)); // (1.0f - p.z)

        buffer.setGlobalTexture(SHADOW_MAP, (p_ptr->m_shadowMap) ? p_ptr->m_shadowMap->depthAttachment() : nullptr);

        buffer.drawMesh(t, mesh, 0, layer, instance);
    }
}
/*!
    \internal
*/
void SpotLight::shadowsUpdate(const Camera &camera, PipelineContext *context, RenderList &components) {
    A_UNUSED(camera);

    if(!castShadows()) {
        p_ptr->m_shadowMap = nullptr;
        return;
    }
    CommandBuffer *buffer = context->buffer();

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
    Matrix4 crop = Matrix4::perspective(p_ptr->m_angle * 2.0f, 1.0f, p_ptr->m_near, zFar);

    int32_t x, y, w, h;
    p_ptr->m_shadowMap = context->requestShadowTiles(uuid(), 1, &x, &y, &w, &h, 1);

    int32_t pageWidth, pageHeight;
    RenderSystem::atlasPageSize(pageWidth, pageHeight);

    p_ptr->m_matrix = scale * crop * rot;
    p_ptr->m_tiles = Vector4(static_cast<float>(x) / pageWidth,
                             static_cast<float>(y) / pageHeight,
                             static_cast<float>(w) / pageWidth,
                             static_cast<float>(h) / pageHeight);

    auto instance = material();
    if(instance) {
        instance->setMatrix4(uni_matrix, &p_ptr->m_matrix);
        instance->setVector4(uni_tiles,  &p_ptr->m_tiles);
    }

    buffer->setRenderTarget(p_ptr->m_shadowMap);
    buffer->enableScissor(x, y, w, h);
    buffer->clearRenderTarget();
    buffer->disableScissor();

    buffer->setViewProjection(rot, crop);
    buffer->setViewport(x, y, w, h);

    RenderList filter = Camera::frustumCulling(components,
                                               Camera::frustumCorners(false, p_ptr->m_angle * 2.0f, 1.0f, pos, q, p_ptr->m_near, zFar));
    // Draw in the depth buffer from position of the light source
    for(auto it : filter) {
        it->draw(*buffer, CommandBuffer::SHADOWCAST);
    }
    buffer->resetViewProjection();
}
/*!
    \internal
*/
AABBox SpotLight::bound() const {
    return p_ptr->m_box * actor()->transform()->worldTransform();
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

    p_ptr->m_box = AABBox(Vector3(0.0f, 0.0f,-0.5f) * distance, Vector3(distance * 1.5f, distance * 1.5f, distance));
}
/*!
    Returns the angle of the light cone in degrees.
*/
float SpotLight::outerAngle() const {
    return p_ptr->m_angle;
}
/*!
    Changes the \a angle of the light cone in degrees.
*/
void SpotLight::setOuterAngle(float angle) {
    p_ptr->m_angle = angle;
    Vector4 p = params();
    p.w = cos(DEG2RAD * p_ptr->m_angle);
    setParams(p);
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

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
