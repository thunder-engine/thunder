#include "pipelinetasks/bloom.h"

#include "engine.h"

#include "resources/rendertarget.h"
#include "resources/material.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "components/private/postprocessorsettings.h"

#include <cmath>
#include <cstring>

#define MAX_SAMPLES 32

namespace {
    const char *gBloom("graphics.bloom");
    const char *gBloomThreshold("bloom/Threshold");

    const char *gRgbMap("rgbMap");

    const char *gDirection("direction");
    const char *gSteps("steps");
    const char *gCurve("curve");
    const char *gSize("size");
    const char *gThreshold("threshold");
};

Bloom::Bloom() :
        m_resultTarget(Engine::objectCreate<RenderTarget>("bloomResultTarget")),
        m_blurTempTarget(Engine::objectCreate<RenderTarget>("blurTempTarget")),
        m_blurTempTexture(Engine::objectCreate<Texture>("blurTempTexture")),
        m_threshold(1.0f) {

    setName("Bloom");

    PostProcessSettings::registerSetting(gBloomThreshold, m_threshold);

    Engine::setValue(gBloom, true);

    Material *blur = Engine::loadResource<Material>(".embedded/Blur.shader");

    m_blurTempTexture->setFormat(Texture::RGB10A2);
    m_blurTempTexture->setFiltering(Texture::Bilinear);
    m_blurTempTexture->setFlags(Texture::Render);

    m_blurTempTarget->setColorAttachment(0, m_blurTempTexture);
    m_blurTempTarget->setClearFlags(RenderTarget::ClearColor);

    if(blur) {
        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            Vector2 direction;

            m_bloomPasses[i].blurMaterialH = blur->createInstance();

            direction.x = 1.0f;
            direction.y = 0.0f;
            m_bloomPasses[i].blurMaterialH->setVector2(gDirection, &direction);
            m_bloomPasses[i].blurMaterialH->setFloat(gThreshold, &m_threshold);

            m_bloomPasses[i].blurMaterialV = blur->createInstance();

            direction.x = 0.0f;
            direction.y = 1.0f;
            m_bloomPasses[i].blurMaterialV->setVector2(gDirection, &direction);
            m_bloomPasses[i].blurMaterialV->setTexture(gRgbMap, m_blurTempTexture);
        }
    }

    m_bloomPasses[0].blurSize = 1.0f;
    m_bloomPasses[1].blurSize = 4.0f;
    m_bloomPasses[2].blurSize = 16.0f;
    m_bloomPasses[3].blurSize = 32.0f;
    m_bloomPasses[4].blurSize = 64.0f;

    m_inputs.push_back("In");
    m_inputs.push_back("Downsample 1/2");
    m_inputs.push_back("Downsample 1/4");
    m_inputs.push_back("Downsample 1/8");
    m_inputs.push_back("Downsample 1/16");
    m_inputs.push_back("Downsample 1/32");

    m_outputs.push_back(std::make_pair("Result", nullptr));
}

Bloom::~Bloom() {
    for(int i = 0; i < BLOOM_PASSES; i++) {
        delete m_bloomPasses[i].blurMaterialH;
        delete m_bloomPasses[i].blurMaterialV;
    }

    m_resultTarget->deleteLater();
    m_blurTempTarget->deleteLater();

    m_blurTempTexture->deleteLater();
}

void Bloom::exec() {
    CommandBuffer *buffer = m_context->buffer();

    float threshold = PostProcessSettings::defaultValue(gBloomThreshold).toFloat();
    for(auto pool : m_context->culledPostEffectSettings()) {
        Variant value = pool.first->readValue(gBloomThreshold);
        if(value.isValid()) {
            threshold = MIX(threshold, value.toFloat(), pool.second);
        }
    }
    if(threshold != m_threshold) {
        m_threshold = threshold;
        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            m_bloomPasses[i].blurMaterialV->setFloat(gThreshold, &m_threshold);
        }
    }

    buffer->beginDebugMarker("Bloom");

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        buffer->setRenderTarget(m_blurTempTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_bloomPasses[i].blurMaterialH);

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_bloomPasses[i].blurMaterialV);
    }

    buffer->endDebugMarker();
}

void Bloom::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        Vector2 size(1.0f / (float)width, 1.0f / (float)height);

        m_blurTempTexture->resize(width, height);

        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            float radius = MAX((width >> i), 1) * m_bloomPasses[i].blurSize * 2.0f * 0.01f;
            int32_t steps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

            float blurPoints[MAX_SAMPLES];
            generateKernel(radius, steps, blurPoints);

            m_bloomPasses[i].blurMaterialH->setInteger(gSteps, &steps);
            m_bloomPasses[i].blurMaterialH->setFloat(gCurve, blurPoints);
            m_bloomPasses[i].blurMaterialH->setVector2(gSize, &size);

            m_bloomPasses[i].blurMaterialV->setInteger(gSteps, &steps);
            m_bloomPasses[i].blurMaterialV->setFloat(gCurve, blurPoints);
            m_bloomPasses[i].blurMaterialV->setVector2(gSize, &size);
        }
    }

    PipelineTask::resize(width, height);
}

void Bloom::setInput(int index, Texture *source) {
    if(index == 0) {
        m_outputs.front().second = source;
        m_resultTarget->setColorAttachment(0, source);
    } else {
        m_bloomPasses[index - 1].blurMaterialH->setTexture(gRgbMap, source);
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
