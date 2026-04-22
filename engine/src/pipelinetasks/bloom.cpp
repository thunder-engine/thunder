#include "pipelinetasks/bloom.h"

#include "engine.h"

#include "resources/rendertarget.h"
#include "resources/material.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "components/private/postprocessorsettings.h"

#include <cmath>
#include <cstring>

namespace {
    const char *gBloom("g.bloom");
    const char *gBloomThreshold("bloom/Threshold");

    const char *gRgbMap("rgbMap");

    const char *gDirection("direction");
    const char *gSteps("steps");
    const char *gCurve("curve");
    const char *gSize("size");

    const char *gThreshold("threshold");
    const char *gLod("lod");
};

Bloom::Bloom() :
        m_resultTarget(Engine::objectCreate<RenderTarget>("bloomResultTarget")),
        m_blurTempTarget(Engine::objectCreate<RenderTarget>("blurTempTarget")),
        m_downTarget(Engine::objectCreate<RenderTarget>("downSampleTarget")),
        m_downTexture(Engine::objectCreate<Texture>("downSampleTexture")),
        m_blurTempTexture(Engine::objectCreate<Texture>("blurTempTexture")) {

    setName("Bloom");

    PostProcessSettings::registerSetting(gBloomThreshold, 1.0f);

    Engine::setValue(gBloom, true);

    m_downTexture->setFormat(Texture::RGB10A2);
    m_downTexture->setFiltering(Texture::Bilinear);
    m_downTexture->setFlags(Texture::Render);

    m_downTarget->setColorAttachment(0, m_downTexture);

    Material *bloomThreshold = Engine::loadResource<Material>(".embedded/BloomThreshold.shader");
    if(bloomThreshold) {
        m_thresholdMaterial = bloomThreshold->createInstance();
    }

    Material *downSample = Engine::loadResource<Material>(".embedded/Downsample.shader");
    if(downSample) {
        m_downMaterial = downSample->createInstance();
        m_downMaterial->setTexture(gRgbMap, m_downTexture);
    }

    m_blurTempTexture->setFormat(Texture::RGB10A2);
    m_blurTempTexture->setFiltering(Texture::Bilinear);
    m_blurTempTexture->setFlags(Texture::Render);

    m_blurTempTarget->setColorAttachment(0, m_blurTempTexture);
    m_blurTempTarget->setClearFlags(RenderTarget::ClearColor);

    Material *blur = Engine::loadResource<Material>(".embedded/Blur.shader");
    if(blur) {
        m_blurMaterial = blur->createInstance();
    }

    m_inputs.push_back("In");

    m_outputs.push_back(std::make_pair("Result", nullptr));
}

Bloom::~Bloom() {
    delete m_thresholdMaterial;
    delete m_downMaterial;
    delete m_blurMaterial;

    m_resultTarget->deleteLater();
    m_blurTempTarget->deleteLater();
    m_downTarget->deleteLater();

    m_downTexture->deleteLater();
    m_blurTempTexture->deleteLater();
}

void Bloom::exec() {
    if(m_mipLevels == 0) {
        return;
    }

    CommandBuffer *buffer = m_context->buffer();

    float threshold = PostProcessSettings::defaultValue(gBloomThreshold).toFloat();
    for(auto pool : m_context->culledPostEffectSettings()) {
        Variant value = pool.first->readValue(gBloomThreshold);
        if(value.isValid()) {
            threshold = MIX(threshold, value.toFloat(), pool.second);
        }
    }

    buffer->beginDebugMarker("Bloom");

    buffer->setViewport(0, 0, m_width, m_height);

    buffer->setRenderTarget(m_downTarget);

    m_thresholdMaterial->setFloat(gThreshold, &threshold);
    buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_thresholdMaterial);

    for(uint8_t i = 1; i < m_mipLevels; i++) {
        buffer->setViewport(0, 0, (m_width >> i), (m_height >> i));

        buffer->setRenderTarget(m_downTarget, i);
        float lod = i-1;
        m_downMaterial->setFloat(gLod, &lod);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_downMaterial);
    }

    Vector2 direction;
    for(uint8_t i = m_mipLevels-1; i >= 1; i--) {
        buffer->setViewport(0, 0, (m_width >> i), (m_height >> i));
        buffer->setRenderTarget(m_blurTempTarget, i);

        float lod = i;
        m_blurMaterial->setFloat(gLod, &lod);
        m_blurMaterial->setInteger(gSteps, &m_bloomPasses[i].steps);
        m_blurMaterial->setFloat(gCurve, m_bloomPasses[i].blurPoints);

        direction.x = 1.0f;
        direction.y = 0.0f;
        m_blurMaterial->setVector2(gDirection, &direction);
        m_blurMaterial->setTexture(gRgbMap, m_downTexture);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_blurMaterial);

        buffer->setViewport(0, 0, m_width, m_height);
        buffer->setRenderTarget(m_resultTarget);

        direction.x = 0.0f;
        direction.y = 1.0f;
        m_blurMaterial->setVector2(gDirection, &direction);
        m_blurMaterial->setTexture(gRgbMap, m_blurTempTexture);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_blurMaterial);
    }

    buffer->endDebugMarker();
}

void Bloom::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        Vector2 size(1.0f / (float)width, 1.0f / (float)height);

        m_mipLevels = std::log2(std::min(width, height));
        m_bloomPasses.resize(m_mipLevels);

        m_blurTempTexture->setMipCount(m_mipLevels);
        m_blurTempTexture->resize(width, height);

        m_downTexture->setMipCount(m_mipLevels);
        m_downTexture->resize(width, height);

        for(uint8_t i = 0; i < m_mipLevels; i++) {
            float blurSize = (1<<i);

            float radius = MAX((width >> i), 1) * blurSize * 2.0f * 0.01f;
            m_bloomPasses[i].steps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

            generateKernel(radius, m_bloomPasses[i].steps, m_bloomPasses[i].blurPoints);

            m_blurMaterial->setVector2(gSize, &size);
        }
    }

    PipelineTask::resize(width, height);
}

void Bloom::setInput(int index, Texture *source) {
    if(index == 0) {
        m_thresholdMaterial->setTexture(gRgbMap, source);

        m_outputs.front().second = source;
        m_resultTarget->setColorAttachment(0, source);
    }
}

void Bloom::generateKernel(float radius, int32_t steps, float *points) {
    memset(points, 0, sizeof(float) * MAX_SAMPLES);

    float total = 0.0f;
    for(uint8_t p = 0; p < steps; p++) {
        float weight = std::exp(-static_cast<float>(p * p) / (2.0f * radius));
        points[p] = weight;

        total += weight;
    }

    for(uint8_t p = 0; p < steps; p++) {
        points[p] *= 1.0f / total * 0.5f; // 1.0 / (sqrt(2.0 * PI) * sigma;
    }
}
