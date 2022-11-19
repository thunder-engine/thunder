#include "postprocess/ambientocclusion.h"

#include "engine.h"

#include "components/private/postprocessorsettings.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "filters/blur.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "amath.h"

#include <cstring>

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
        m_noiseTexture(nullptr),
        m_ssaoTexture(nullptr),
        m_blur(nullptr),
        m_combine(nullptr) {

    for(int32_t i = 0; i < KERNEL_SIZE; i++) {
        m_samplesKernel[i].x = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
        m_samplesKernel[i].y = RANGE(0.0f, 1.0f) * 2.0f - 1.0f;
        m_samplesKernel[i].z = RANGE(0.0f, 1.0f);

        m_samplesKernel[i].normalize();
        m_samplesKernel[i] *= RANGE(0.0f, 1.0f);

        float scale = static_cast<float>(i) / KERNEL_SIZE;
        scale = MIX(0.1f, 1.0f, scale * scale);
        m_samplesKernel[i] *= scale;
    }

    m_noiseTexture = Engine::objectCreate<Texture>();
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

    m_ssaoTexture = Engine::objectCreate<Texture>();
    m_ssaoTexture->setFormat(Texture::R8);

    m_ssaoTarget = Engine::objectCreate<RenderTarget>();
    m_ssaoTarget->setColorAttachment(0, m_ssaoTexture);

    m_resultTexture = Engine::objectCreate<Texture>();
    m_resultTexture->setFormat(Texture::R8);

    m_resultTarget = Engine::objectCreate<RenderTarget>();

    m_blurTarget = Engine::objectCreate<RenderTarget>();
    m_blurTarget->setColorAttachment(0, m_resultTexture);

    {
        Material *mtl = Engine::loadResource<Material>(".embedded/AmbientOcclusion.shader");
        if(mtl) {
            m_material = mtl->createInstance();
            m_material->setVector3("uni.samplesKernel", m_samplesKernel, KERNEL_SIZE);

            m_material->setTexture("noiseMap", m_noiseTexture);
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
            m_combine->setTexture("ssaoMap", m_resultTexture);
        }
    }

    setName("AmbientOcclusion");

    Engine::setValue(ambientOcclusion, true);

    PostProcessSettings::registerSetting(ambientRadius, m_radius);
    PostProcessSettings::registerSetting(ambientBias, m_bias);
    PostProcessSettings::registerSetting(ambientPower, m_power);

}

AmbientOcclusion::~AmbientOcclusion() {
    m_noiseTexture->deleteLater();
}

Texture *AmbientOcclusion::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();
    if(m_material) {
        buffer->setViewport(0, 0, m_ssaoTexture->width(), m_ssaoTexture->height());

        buffer->setRenderTarget(m_ssaoTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_material);

        context->setRenderTexture("SSAOSample", m_ssaoTexture);
    }

    if(m_blur) {
        buffer->setViewport(0, 0, m_resultTexture->width(), m_resultTexture->height());

        buffer->setRenderTarget(m_blurTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_blur);

        context->setRenderTexture("SSAOBlur", m_resultTexture);
    }

    if(m_combine) {
        m_resultTarget->setColorAttachment(0, source);

        buffer->setViewport(0, 0, source->width(), source->height());

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_combine);

        context->setRenderTexture("SSAOCombine", source);
    }

    return source;
}

void AmbientOcclusion::resize(int32_t width, int32_t height) {
    m_ssaoTexture->setWidth(width);
    m_ssaoTexture->setHeight(height);

    m_resultTexture->setWidth(width);
    m_resultTexture->setHeight(height);
}

void AmbientOcclusion::setSettings(const PostProcessSettings &settings) {
    m_radius = settings.readValue(ambientRadius).toFloat();
    m_bias = settings.readValue(ambientBias).toFloat();
    m_power = settings.readValue(ambientPower).toFloat();

    if(m_material) {
        m_material->setFloat("uni.radius", &m_radius);
        m_material->setFloat("uni.bias", &m_bias);
        m_material->setFloat("uni.power", &m_power);
    }
}

uint32_t AmbientOcclusion::layer() const {
    return CommandBuffer::DEFAULT;
}
