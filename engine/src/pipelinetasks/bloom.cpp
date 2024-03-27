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
    const char *bloom("graphics.bloom");
    const char *bloomThreshold("bloom/Threshold");

    const char *rgbMap("rgbMap");
};

Bloom::Bloom() :
        m_resultTarget(Engine::objectCreate<RenderTarget>("bloomResultTarget")),
        m_blurTempTarget(Engine::objectCreate<RenderTarget>("blurTempTarget")),
        m_blurTempTexture(Engine::objectCreate<Texture>("blurTempTexture")),
        m_threshold(1.0f) {

    setName("Bloom");

    PostProcessSettings::registerSetting(bloomThreshold, m_threshold);
    Engine::setValue(bloom, true);

    Material *downSample = Engine::loadResource<Material>(".embedded/Downsample.shader");
    Material *blur = Engine::loadResource<Material>(".embedded/Blur.shader");

    m_blurTempTexture->setFormat(Texture::R11G11B10Float);
    m_blurTempTexture->setFiltering(Texture::Bilinear);
    m_blurTempTexture->setFlags(Texture::Render);

    m_blurTempTarget->setColorAttachment(0, m_blurTempTexture);

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        // Downsample
        m_bloomPasses[i].downTexture = Engine::objectCreate<Texture>("downSampleTexture");
        m_bloomPasses[i].downTexture->setFormat(Texture::R11G11B10Float);
        m_bloomPasses[i].downTexture->setFiltering(Texture::Bilinear);
        m_bloomPasses[i].downTexture->setFlags(Texture::Render);

        m_bloomPasses[i].downTarget = Engine::objectCreate<RenderTarget>("downSampleTarget");
        m_bloomPasses[i].downTarget->setColorAttachment(0, m_bloomPasses[i].downTexture);

        if(downSample) {
            m_bloomPasses[i].downMaterial = downSample->createInstance();
            m_bloomPasses[i].downMaterial->setFloat("threshold", &m_threshold);
            if(i > 0) {
                m_bloomPasses[i].downMaterial->setTexture(rgbMap, m_bloomPasses[i - 1].downTexture);
            }
        }

        // Blur
        if(blur) {
            Vector2 direction;

            m_bloomPasses[i].blurMaterialH = blur->createInstance();

            direction.x = 1.0f;
            direction.y = 0.0f;
            m_bloomPasses[i].blurMaterialH->setVector2("direction", &direction);
            m_bloomPasses[i].blurMaterialH->setTexture(rgbMap, m_bloomPasses[i].downTexture);

            m_bloomPasses[i].blurMaterialV = blur->createInstance();

            direction.x = 0.0f;
            direction.y = 1.0f;
            m_bloomPasses[i].blurMaterialV->setVector2("direction", &direction);
            m_bloomPasses[i].blurMaterialV->setTexture(rgbMap, m_blurTempTexture);
        }
    }

    m_bloomPasses[0].blurSize = 1.0f;
    m_bloomPasses[1].blurSize = 4.0f;
    m_bloomPasses[2].blurSize = 16.0f;
    m_bloomPasses[3].blurSize = 32.0f;
    m_bloomPasses[4].blurSize = 64.0f;

    m_inputs.push_back("In");
    m_outputs.push_back(make_pair("Result", nullptr));
}

void Bloom::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();

    Texture *texture(m_outputs.front().second);
    m_bloomPasses[0].downMaterial->setTexture(rgbMap, texture);
    m_resultTarget->setColorAttachment(0, texture);

    buffer->beginDebugMarker("Bloom");

    buffer->beginDebugMarker("Downsample");

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        buffer->setViewport(0, 0, m_bloomPasses[i].downTexture->width(), m_bloomPasses[i].downTexture->height());

        buffer->setRenderTarget(m_bloomPasses[i].downTarget);

        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_bloomPasses[i].downMaterial);
    }

    buffer->setViewport(0, 0, texture->width(), texture->height());

    buffer->endDebugMarker();

    buffer->beginDebugMarker("Combine");

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        buffer->setRenderTarget(m_blurTempTarget);
        buffer->clearRenderTarget(true, Vector4(0.0f), false, 1.0f);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_bloomPasses[i].blurMaterialH);

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_bloomPasses[i].blurMaterialV);
    }

    buffer->endDebugMarker();

    buffer->endDebugMarker();
}

void Bloom::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        Vector2 size(1.0f / (float)width, 1.0f / (float)height);

        m_blurTempTexture->resize(width, height);

        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            int32_t sizeW = MAX((width >> i), 1);
            int32_t sizeH = MAX((height >> i), 1);

            float radius = sizeW * m_bloomPasses[i].blurSize * 2.0f * 0.01f;
            int32_t steps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

            float blurPoints[MAX_SAMPLES];
            generateKernel(radius, steps, blurPoints);

            m_bloomPasses[i].downTexture->resize(sizeW, sizeH);

            m_bloomPasses[i].blurMaterialH->setInteger("steps", &steps);
            m_bloomPasses[i].blurMaterialH->setFloat("curve", blurPoints);
            m_bloomPasses[i].blurMaterialH->setVector2("size", &size);

            m_bloomPasses[i].blurMaterialV->setInteger("steps", &steps);
            m_bloomPasses[i].blurMaterialV->setFloat("curve", blurPoints);
            m_bloomPasses[i].blurMaterialV->setVector2("size", &size);
        }
    }

    PipelineTask::resize(width, height);
}

void Bloom::setSettings(const PostProcessSettings &settings) {
    m_threshold = settings.readValue(bloomThreshold).toFloat();

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        m_bloomPasses[i].downMaterial->setFloat("threshold", &m_threshold);
    }
}

void Bloom::setInput(int index, Texture *source) {
    m_outputs.front().second = source;
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
