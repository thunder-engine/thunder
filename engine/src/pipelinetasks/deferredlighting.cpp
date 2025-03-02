#include "pipelinetasks/deferredlighting.h"

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

namespace {
    const char *gUniPosition("position");
    const char *gUniDirection("direction");
    const char *gUniRight("right");
    const char *gUniUp("up");
}

DeferredLighting::DeferredLighting() :
        m_lightPass(Engine::objectCreate<RenderTarget>("lightPass")) {

    setName("DeferredLighting");

    m_inputs.push_back("In");
    m_outputs.push_back(std::make_pair("Result", nullptr));
}

DeferredLighting::~DeferredLighting() {
    m_lightPass->deleteLater();
}

void DeferredLighting::exec() {
    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("DeferredLighting");

    buffer->setRenderTarget(m_lightPass);

    for(auto it : m_context->sceneLights()) {
        BaseLight *light = static_cast<BaseLight *>(it);

        Mesh *mesh = PipelineContext::defaultCube();

        switch(light->lightType()) {
        case BaseLight::AreaLight: {
            auto instance = light->material();
            if(instance) {
                Matrix4 m(light->transform()->worldTransform());

                Vector3 position(m[12], m[13], m[14]);

                float d = static_cast<AreaLight *>(light)->radius() * 2.0f;

                Vector3 direction(m.rotation() * Vector3(0.0f, 0.0f, 1.0f));
                Vector3 right(m.rotation() * Vector3(1.0f, 0.0f, 0.0f));
                Vector3 up(m.rotation() * Vector3(0.0f, 1.0f, 0.0f));

                instance->setTransform(Matrix4(position, Quaternion(), Vector3(d)));
                instance->setVector3(gUniPosition, &position);
                instance->setVector3(gUniDirection, &direction);
                instance->setVector3(gUniRight, &right);
                instance->setVector3(gUniUp, &up);
            }
        } break;
        case BaseLight::PointLight: {
            auto instance = light->material();
            if(instance) {
                Matrix4 m(light->transform()->worldTransform());

                float d = static_cast<PointLight *>(light)->attenuationRadius() * 2.0f;

                Vector3 position(m[12], m[13], m[14]);
                Vector3 direction(m.rotation() * Vector3(0.0f, 1.0f, 0.0f));

                instance->setTransform(Matrix4(position, Quaternion(), Vector3(d)));
                instance->setVector3(gUniPosition, &position);
                instance->setVector3(gUniDirection, &direction);
            }
        } break;
        case BaseLight::SpotLight: {
            auto instance = light->material();
            if(instance) {
                Transform *t = light->transform();

                Matrix4 m(t->worldTransform());
                Quaternion q(t->worldQuaternion());

                Vector3 position(m[12], m[13], m[14]);
                Vector3 direction(q * Vector3(0.0f, 0.0f, 1.0f));

                float distance = static_cast<SpotLight *>(light)->attenuationDistance();
                float angle = static_cast<SpotLight *>(light)->outerAngle();
                float radius = tan(DEG2RAD * angle * 0.5f) * distance;
                Matrix4 mat(position - direction * distance * 0.5f,
                            q,
                            Vector3(radius * 2.0f, radius * 2.0f, distance));

                instance->setTransform(mat);
                instance->setVector3(gUniPosition, &position);
                instance->setVector3(gUniDirection, &direction);
            }
        } break;
        case BaseLight::DirectLight: {
            mesh = PipelineContext::defaultPlane();

            auto instance = light->material();
            if(instance) {
                Transform *t = light->transform();
                m_sunDirection = Vector3(t->worldQuaternion() * Vector3(0.0f, 0.0f, 1.0f));

                instance->setVector3(gUniDirection, &m_sunDirection);
            }
        } break;
        default: break;
        }

        buffer->drawMesh(mesh, 0, CommandBuffer::LIGHT, *light->material());
    }
    buffer->endDebugMarker();
}

void DeferredLighting::setInput(int index, Texture *texture) {
    m_lightPass->setColorAttachment(0, texture);

    m_outputs.front().second = texture;
}
