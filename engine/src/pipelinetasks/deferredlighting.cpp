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
    const char *uniPosition  = "position";
    const char *uniDirection = "direction";
    const char *uniRight     = "right";
    const char *uniUp        = "up";
}

DeferredLighting::DeferredLighting() :
    m_lightPass(Engine::objectCreate<RenderTarget>("lightPass")) {

    m_inputs.push_back("In");
    m_outputs.push_back(make_pair("Result", nullptr));
}

void DeferredLighting::exec(PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();
    buffer->beginDebugMarker("DeferredLighting");

    buffer->setRenderTarget(m_lightPass);
    // Light pass
    for(auto it : context->sceneLights()) {
        BaseLight *light = static_cast<BaseLight *>(it);

        Mesh *mesh = PipelineContext::defaultCube();

        Matrix4 mat;
        switch(light->lightType()) {
        case BaseLight::AreaLight: {
            Matrix4 m = light->transform()->worldTransform();

            Vector3 position(m[12], m[13], m[14]);

            float d = static_cast<AreaLight *>(light)->radius() * 2.0f;
            mat = Matrix4(position, Quaternion(), Vector3(d));

            auto instance = light->material();
            if(instance) {
                Vector3 direction(m.rotation() * Vector3(0.0f, 0.0f, 1.0f));
                Vector3 right(m.rotation() * Vector3(1.0f, 0.0f, 0.0f));
                Vector3 up(m.rotation() * Vector3(0.0f, 1.0f, 0.0f));

                instance->setVector3(uniPosition, &position);
                instance->setVector3(uniDirection, &direction);
                instance->setVector3(uniRight, &right);
                instance->setVector3(uniUp, &up);
            }
        } break;
        case BaseLight::PointLight: {
            Matrix4 m = light->transform()->worldTransform();

            float d = static_cast<PointLight *>(light)->attenuationRadius() * 2.0f;
            mat = Matrix4(Vector3(m[12], m[13], m[14]), Quaternion(), Vector3(d));

            auto instance = light->material();
            if(instance) {
                Vector3 position(m[12], m[13], m[14]);
                Vector3 direction(m.rotation() * Vector3(0.0f, 1.0f, 0.0f));

                instance->setVector3(uniPosition, &position);
                instance->setVector3(uniDirection, &direction);
            }
        } break;
        case BaseLight::SpotLight: {
            Transform *t = light->transform();

            Matrix4 m(t->worldTransform());
            Quaternion q(t->worldQuaternion());

            Vector3 position(m[12], m[13], m[14]);
            Vector3 direction(q * Vector3(0.0f, 0.0f,-1.0f));

            float distance = static_cast<SpotLight *>(light)->attenuationDistance();
            float angle = static_cast<SpotLight *>(light)->outerAngle();
            float radius = tan(DEG2RAD * angle * 0.5f) * distance;
            mat = Matrix4(position - direction * distance * 0.5f,
                          q,
                          Vector3(radius * 2.0f, radius * 2.0f, distance));

            auto instance = light->material();
            if(instance) {
                instance->setVector3(uniPosition, &position);
                instance->setVector3(uniDirection, &direction);
            }
        } break;
        case BaseLight::DirectLight: {
            mesh = PipelineContext::defaultPlane();

            auto instance = light->material();
            if(instance) {
                Transform *t = light->transform();
                Vector3 direction(t->worldQuaternion() * Vector3(0.0f, 0.0f, 1.0f));

                instance->setVector3(uniDirection, &direction);
            }
        } break;
        default: break;
        }

        buffer->drawMesh(mat, mesh, 0, CommandBuffer::LIGHT, light->material());
    }
    buffer->endDebugMarker();
}

void DeferredLighting::setInput(int index, Texture *texture) {
    m_lightPass->setColorAttachment(0, texture);

    m_outputs.back().second = texture;
}
