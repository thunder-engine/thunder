#include "postprocess/ambientocclusion.h"

#include "engine.h"

#include "components/scene.h"

#include "resources/pipeline.h"
#include "resources/material.h"
#include "resources/rendertarget.h"

#include "filters/blur.h"

#include "commandbuffer.h"

#include "amath.h"

#include <cstring>

#define BLUR_STEPS 4

#define SSAO_MAP    "uni.ssaoMap"
#define RGB_MAP     "uni.rgbMap"

namespace {
const char *AMBIENT_OCCLUSION("Graphics/Advanced/AmbientOcclusion");

const char *AMBIENT_RADIUS("ambientOcclusion/Radius");
const char *AMBIENT_BIAS("ambientOcclusion/Bias");
const char *AMBIENT_POWER("ambientOcclusion/Power");
};

AmbientOcclusion::AmbientOcclusion() :
        m_radius(0.2f),
        m_bias(0.025f),
        m_power(2.0f),
        m_noiseTexture(nullptr),
        m_ssaoTexture(nullptr),
        m_blur(nullptr),
        m_occlusion(nullptr) {

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

    m_blurTarget = Engine::objectCreate<RenderTarget>();
    m_blurTarget->setColorAttachment(0, m_resultTexture);

    {
        Material *mtl = Engine::loadResource<Material>(".embedded/AmbientOcclusion.mtl");
        if(mtl) {
            m_material = mtl->createInstance();
            m_material->setVector3("samplesKernel", m_samplesKernel, KERNEL_SIZE);

            m_material->setFloat("radius", &m_radius);
            m_material->setFloat("bias", &m_bias);
            m_material->setFloat("power", &m_power);

            m_material->setTexture("noiseMap", m_noiseTexture);
        }
    }
    {
        Material *mtl = Engine::loadResource<Material>(".embedded/BlurOcclusion.mtl");
        if(mtl) {
            m_blur = mtl->createInstance();
            m_blur->setTexture("ssaoSample", m_ssaoTexture);
        }
    }
    {
        Material *mtl = Engine::loadResource<Material>(".embedded/CombineOcclusion.mtl");
        if(mtl) {
            m_occlusion = mtl->createInstance();
            m_occlusion->setTexture(SSAO_MAP, m_resultTexture);
        }
    }

    Engine::setValue(AMBIENT_OCCLUSION, true);

    PostProcessSettings::registerSetting(AMBIENT_RADIUS, m_radius);
    PostProcessSettings::registerSetting(AMBIENT_BIAS, m_bias);
    PostProcessSettings::registerSetting(AMBIENT_POWER, m_power);

}

AmbientOcclusion::~AmbientOcclusion() {
    m_noiseTexture->deleteLater();
}

Texture *AmbientOcclusion::draw(Texture *source, Pipeline *pipeline) {
    if(m_enabled) {
        ICommandBuffer *buffer = pipeline->buffer();
        if(m_material) {
            buffer->setViewport(0, 0, m_ssaoTexture->width(), m_ssaoTexture->height());

            buffer->setRenderTarget(m_ssaoTarget);
            buffer->drawMesh(Matrix4(), m_mesh, ICommandBuffer::UI, m_material);
        }

        if(m_blur) {
            buffer->setViewport(0, 0, m_resultTexture->width(), m_resultTexture->height());

            buffer->setRenderTarget(m_blurTarget);
            buffer->drawMesh(Matrix4(), m_mesh, ICommandBuffer::UI, m_blur);
        }

        if(m_occlusion) {
            m_resultTarget->setColorAttachment(0, source);

            buffer->setViewport(0, 0, source->width(), source->height());

            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(Matrix4(), m_mesh, ICommandBuffer::UI, m_occlusion);
        }
    }
    return source;
}

void AmbientOcclusion::resize(int32_t width, int32_t height) {
    m_ssaoTexture->setWidth(width);
    m_ssaoTexture->setHeight(height);

    m_resultTexture->setWidth(width);
    m_resultTexture->setHeight(height);

    float radius = width * 0.01f;
    memset(m_blurSamplesKernel, 0, sizeof(float) * BLUR_STEPS);
    Blur::generateKernel(radius, BLUR_STEPS, m_blurSamplesKernel);
}

void AmbientOcclusion::setSettings(const PostProcessSettings &settings) {
    m_radius = settings.readValue(AMBIENT_RADIUS).toFloat();
    m_bias = settings.readValue(AMBIENT_BIAS).toFloat();
    m_power = settings.readValue(AMBIENT_POWER).toFloat();
}

uint32_t AmbientOcclusion::layer() const {
    return ICommandBuffer::DEFAULT;
}

const char *AmbientOcclusion::name() const {
    return "ScreenSpaceAmbientOcclusion";
}
