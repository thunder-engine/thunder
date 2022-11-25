#include "pipelinepasses/deferredlighting.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"
#include "components/arealight.h"
#include "components/pointlight.h"
#include "components/spotlight.h"

#include "resources/rendertarget.h"
#include "resources/mesh.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

DeferredLighting::DeferredLighting() :
    m_lightPass(Engine::objectCreate<RenderTarget>(LIGHPASS)),
    m_box(Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001")) {

}

Texture *DeferredLighting::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();

    buffer->setRenderTarget(m_lightPass);
    // Light pass
    for(auto it : context->sceneLights()) {
        BaseLight *light = static_cast<BaseLight *>(it);

        Mesh *mesh = m_box;

        Matrix4 mat;
        switch(light->lightType()) {
        case BaseLight::AreaLight: {
            Matrix4 m = light->actor()->transform()->worldTransform();

            float d = static_cast<AreaLight *>(light)->radius() * 2.0f;
            mat = Matrix4(Vector3(m[12], m[13], m[14]), Quaternion(), Vector3(d));
        } break;
        case BaseLight::PointLight: {
            Matrix4 m = light->actor()->transform()->worldTransform();

            float d = static_cast<PointLight *>(light)->attenuationRadius() * 2.0f;
            mat = Matrix4(Vector3(m[12], m[13], m[14]), Quaternion(), Vector3(d));
        } break;
        case BaseLight::SpotLight: {
            Transform *t = light->actor()->transform();

            Matrix4 m(t->worldTransform());
            Quaternion q(t->worldQuaternion());

            Vector3 direction(q * Vector3(0.0f, 0.0f, 1.0f));

            float d = static_cast<SpotLight *>(light)->attenuationDistance();
            mat = Matrix4(Vector3(m[12], m[13], m[14]) - direction * d * 0.5f, q, Vector3(d * 1.5f, d * 1.5f, d));
        } break;
        case BaseLight::DirectLight: {
            mesh = PipelineContext::defaultPlane();
            buffer->setScreenProjection();
        } break;
        default: break;
        }

        buffer->drawMesh(mat, mesh, 0, CommandBuffer::LIGHT, light->material());

        if(light->lightType() == BaseLight::DirectLight) {
            buffer->resetViewProjection();
        }
    }

    // Transparent pass
    context->drawRenderers(CommandBuffer::TRANSLUCENT, context->culledComponents());

    return source;
}

void DeferredLighting::setInput(uint32_t index, Texture *texture) {
    switch(index) {
    case Emissve: m_lightPass->setColorAttachment(Emissve, texture); break;
    case Depth: m_lightPass->setDepthAttachment(texture); break;
    default: break;
    }
}

uint32_t DeferredLighting::layer() const {
    return CommandBuffer::LIGHT;
}
