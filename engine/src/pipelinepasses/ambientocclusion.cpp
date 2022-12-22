#include "pipelinepasses/ambientocclusion.h"

#include "engine.h"

#include "components/private/postprocessorsettings.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "filters/blur.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "amath.h"

#include <cstring>

#define KERNEL_SIZE 16

namespace {
    const char *ambientOcclusion("graphics.ambientocclusion");

    const char *ambientRadius("ambientOcclusion/Radius");
    const char *ambientBias("ambientOcclusion/Bias");
    const char *ambientPower("ambientOcclusion/Power");
};

AmbientOcclusion::AmbientOcclusion() :
        m_radius(0.2f),
        m_bias(0.025f),
        m_power(2.0f),
        m_noiseTexture(Engine::objectCreate<Texture>()),
        m_ssaoTexture(Engine::objectCreate<Texture>("SSAO")),
        m_blurTexture(Engine::objectCreate<Texture>("SSAOBlur")),
        m_ssaoTarget(Engine::objectCreate<RenderTarget>()),
        m_blurTarget(Engine::objectCreate<RenderTarget>()),
        m_combineTarget(Engine::objectCreate<RenderTarget>()),
        m_occlusion(nullptr),
        m_blur(nullptr),
        m_combine(nullptr) {

    setName("AmbientOcclusion");

    Engine::setValue(ambientOcclusion, true);

    PostProcessSettings::registerSetting(ambientRadius, m_radius);
    PostProcessSettings::registerSetting(ambientBias, m_bias);
    PostProcessSettings::registerSetting(ambientPower, m_power);

    m_noiseTexture->setFormat(Texture::RGB16Float);
    m_noiseTexture->setWrap(Texture::Repeat);
    m_noiseTexture->setFiltering(Texture::None);
    m_noiseTexture->resize(4, 4);

    Texture::Surface &s = m_noiseTexture->surface(0);

    Vector3 *ptr = reinterpret_cast<Vector3 *>(&(s[0])[0]);
    for(int32_t i = 0; i < KERNEL_SIZE; i++) {
        ptr[i].x = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
        ptr[i].y = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
        ptr[i].z = 0.0f;

        ptr[i].normalize();
    }

    m_ssaoTexture->setFormat(Texture::R8);
    m_blurTexture->setFormat(Texture::R8);

    m_ssaoTarget->setColorAttachment(0, m_ssaoTexture);
    m_blurTarget->setColorAttachment(0, m_blurTexture);

    {
        Material *mtl = Engine::loadResource<Material>(".embedded/AmbientOcclusion.shader");
        if(mtl) {
            m_occlusion = mtl->createInstance();

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
            m_occlusion->setVector3("uni.samplesKernel", samplesKernel, KERNEL_SIZE);
            m_occlusion->setTexture("noiseMap", m_noiseTexture);
        }
    }
    {
        Material *mtl = Engine::loadResource<Material>(".embedded/BlurOcclusion.shader");
        if(mtl) {
            m_blur = mtl->createInstance();
            m_blur->setTexture("ssaoSample", m_ssaoTexture);
        }
    }
    {
        Material *mtl = Engine::loadResource<Material>(".embedded/CombineOcclusion.shader");
        if(mtl) {
            m_combine = mtl->createInstance();
            m_combine->setTexture("ssaoMap", m_blurTexture);
        }
    }
}

AmbientOcclusion::~AmbientOcclusion() {
    m_noiseTexture->deleteLater();
}

Texture *AmbientOcclusion::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();
    if(m_occlusion) {
        buffer->setViewport(0, 0, m_ssaoTexture->width(), m_ssaoTexture->height());

        buffer->setRenderTarget(m_ssaoTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_occlusion);
    }

    if(m_blur) {
        buffer->setViewport(0, 0, m_blurTexture->width(), m_blurTexture->height());

        buffer->setRenderTarget(m_blurTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_blur);
    }

    if(m_combine) {
        Texture *texture = m_combineTarget->colorAttachment(Input);
        if(texture) {
            buffer->setViewport(0, 0, texture->width(), texture->height());
        }

        buffer->setRenderTarget(m_combineTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_combine);
    }
    return source;
}

void AmbientOcclusion::resize(int32_t width, int32_t height) {
    m_ssaoTexture->setWidth(width);
    m_ssaoTexture->setHeight(height);

    m_blurTexture->setWidth(width);
    m_blurTexture->setHeight(height);
}

void AmbientOcclusion::setSettings(const PostProcessSettings &settings) {
    m_radius = settings.readValue(ambientRadius).toFloat();
    m_bias = settings.readValue(ambientBias).toFloat();
    m_power = settings.readValue(ambientPower).toFloat();

    if(m_occlusion) {
        m_occlusion->setFloat("uni.radius", &m_radius);
        m_occlusion->setFloat("uni.bias", &m_bias);
        m_occlusion->setFloat("uni.power", &m_power);
    }
}

uint32_t AmbientOcclusion::layer() const {
    return CommandBuffer::DEFAULT;
}

void AmbientOcclusion::setInput(uint32_t index, Texture *texture) {
    if(index == Input)  {
        m_combineTarget->setColorAttachment(Input, texture);
    }
}

uint32_t AmbientOcclusion::outputCount() const {
    return 2;
}

Texture *AmbientOcclusion::output(uint32_t index) {
    switch(index) {
        case 0: return m_ssaoTexture;
        case 1: return m_blurTexture;
        default: break;
    }
    return nullptr;
}
