#include "pipelinetasks/ambientocclusion.h"

#include "components/private/postprocessorsettings.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include <cstring>

#define KERNEL_SIZE 16

namespace {
    const char *ambientOcclusion("graphics.ambientocclusion");

    const char *ambientRadius("ambientOcclusion/radius");
    const char *ambientBias("ambientOcclusion/bias");
    const char *ambientPower("ambientOcclusion/power");
};

AmbientOcclusion::AmbientOcclusion() :
        m_radius(0.2f),
        m_bias(0.025f),
        m_power(2.0f),
        m_noiseTexture(Engine::objectCreate<Texture>("aoNoiseTexture")),
        m_aoTexture(Engine::objectCreate<Texture>("aoOcclusion")),
        m_blurTexture(Engine::objectCreate<Texture>("aoBlur")),
        m_aoTarget(Engine::objectCreate<RenderTarget>("aoTarget")),
        m_blurTarget(Engine::objectCreate<RenderTarget>("aoBlurTarget")),
        m_occlusion(nullptr),
        m_blur(nullptr) {

    setName("AmbientOcclusion");

    m_inputs.push_back("In");
    m_outputs.push_back(std::make_pair("Result", nullptr));

    Engine::setValue(ambientOcclusion, true);

    PostProcessSettings::registerSetting(ambientRadius, m_radius);
    PostProcessSettings::registerSetting(ambientBias, m_bias);
    PostProcessSettings::registerSetting(ambientPower, m_power);

    m_noiseTexture->setFormat(Texture::RGBA32Float);
    m_noiseTexture->setWrap(Texture::Repeat);
    m_noiseTexture->setFiltering(Texture::None);
    m_noiseTexture->resize(4, 4);

    Texture::Surface &s = m_noiseTexture->surface(0);

    Vector4 *ptr = reinterpret_cast<Vector4 *>(&(s[0])[0]);
    for(int32_t i = 0; i < KERNEL_SIZE; i++) {
        ptr[i].x = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
        ptr[i].y = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
        ptr[i].z = 0.0f;
        ptr[i].w = 0.0f;

        ptr[i].normalize();
    }

    m_aoTexture->setFormat(Texture::R8);
    m_aoTexture->setFlags(Texture::Render);

    m_aoTarget->setColorAttachment(0, m_aoTexture);

    m_blurTexture->setFormat(Texture::R8);
    m_blurTexture->setFlags(Texture::Render);

    m_blurTarget->setColorAttachment(0, m_blurTexture);

    {
        Material *material = Engine::loadResource<Material>(".embedded/SSAO.shader");
        if(material) {
            m_occlusion = material->createInstance();

            Vector3 samplesKernel[KERNEL_SIZE];
            for(int32_t i = 0; i < KERNEL_SIZE; i++) {
                samplesKernel[i].x = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
                samplesKernel[i].y = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
                samplesKernel[i].z = RANGE(0.0f, 1.0f);

                samplesKernel[i].normalize();
                samplesKernel[i] *= RANGE(0.0f, 1.0f);

                float scale = static_cast<float>(i) / KERNEL_SIZE;
                scale = MIX(0.1f, 1.0f, scale * scale);
                samplesKernel[i] *= scale;
            }
            m_occlusion->setVector3("samplesKernel", samplesKernel, KERNEL_SIZE);
            m_occlusion->setTexture("noiseMap", m_noiseTexture);

            m_occlusion->setFloat("radius", &m_radius);
            m_occlusion->setFloat("bias", &m_bias);
            m_occlusion->setFloat("power", &m_power);
        }
    }
    {
        Material *material = Engine::loadResource<Material>(".embedded/BlurOcclusion.shader");
        if(material) {
            m_blur = material->createInstance();
            m_blur->setTexture("aoMap", m_aoTexture);
        }
    }

    m_outputs.push_back(make_pair(m_blurTexture->name(), m_blurTexture));
}

AmbientOcclusion::~AmbientOcclusion() {
    m_noiseTexture->deleteLater();
    m_aoTexture->deleteLater();
    m_blurTexture->deleteLater();

    m_aoTarget->deleteLater();
    m_blurTarget->deleteLater();
}

void AmbientOcclusion::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();

    float radius = PostProcessSettings::defaultValue(ambientRadius).toFloat();
    for(auto pool : context.culledPostEffectSettings()) {
        const PostProcessSettings *settings = pool.first;
        Variant value = settings->readValue(ambientRadius);
        if(value.isValid()) {
            radius = MIX(m_radius, value.toFloat(), pool.second);
        }
    }
    if(radius != m_radius) {
        m_radius = radius;
        if(m_occlusion) {
            m_occlusion->setFloat("radius", &m_radius);
        }
    }

    float bias = PostProcessSettings::defaultValue(ambientBias).toFloat();
    for(auto pool : context.culledPostEffectSettings()) {
        Variant value = pool.first->readValue(ambientBias);
        if(value.isValid()) {
            bias = MIX(bias, value.toFloat(), pool.second);
        }
    }
    if(bias != m_radius) {
        m_bias = bias;
        if(m_occlusion) {
            m_occlusion->setFloat("bias", &m_bias);
        }
    }

    float power = PostProcessSettings::defaultValue(ambientPower).toFloat();
    for(auto pool : context.culledPostEffectSettings()) {
        Variant value = pool.first->readValue(ambientPower);
        if(value.isValid()) {
            power = MIX(power, value.toFloat(), pool.second);
        }
    }
    if(power != m_power) {
        m_power = power;
        if(m_occlusion) {
            m_occlusion->setFloat("power", &m_power);
        }
    }

    buffer->beginDebugMarker("AmbientOcclusion");

    if(m_occlusion) {
        buffer->setViewport(0, 0, m_aoTexture->width(), m_aoTexture->height());

        buffer->setRenderTarget(m_aoTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_occlusion);
    }

    if(m_blur) {
        buffer->setViewport(0, 0, m_blurTexture->width(), m_blurTexture->height());

        buffer->setRenderTarget(m_blurTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, CommandBuffer::UI, *m_blur);
    }

    buffer->endDebugMarker();
}

void AmbientOcclusion::resize(int32_t width, int32_t height) {
    PipelineTask::resize(width, height);

    m_aoTexture->resize(width, height);
}

void AmbientOcclusion::setInput(int index, Texture *texture) {
    m_outputs.front().second = texture;
}
