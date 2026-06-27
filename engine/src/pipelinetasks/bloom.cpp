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

    const char *gBloomThreshold("bloom/threshold");
    const char *gBloomIntensity("bloom/intensity");
    const char *gBloomDirt("bloom/dirt");
    const char *gBloomDirtIntensity("bloom/dirtIntensity");

    const char *gRgbMap("rgbMap");

    const char *gDirection("direction");
    const char *gSteps("steps");
    const char *gCurve("curve");
    const char *gSize("size");
    const char *gIntensity("intensity");
    const char *gDirtIntensity("dirtIntensity");
    const char *gThreshold("threshold");
};

Bloom::Bloom() :
        m_resultTarget(Engine::objectCreate<RenderTarget>("bloomResultTarget")),
        m_mipLevels(1),
        m_threshold(1.0f),
        m_intensity(1.0f),
        m_dirtIntensity(0.0f) {

    setName("Bloom");

    Engine::setValue(gBloom, m_enabled);

    PostProcessSettings::registerSetting(gBloomThreshold, m_threshold);
    PostProcessSettings::registerSetting(gBloomIntensity, m_intensity);
    PostProcessSettings::registerSetting(gBloomDirt, Variant::fromValue(m_dirtTexture));
    PostProcessSettings::registerSetting(gBloomDirtIntensity, m_dirtIntensity);

    Material *bloomThreshold = Engine::loadResource<Material>(".embedded/BloomThreshold.shader");
    if(bloomThreshold) {
        m_thresholdMaterial = bloomThreshold->createInstance();
        m_thresholdMaterial->setFloat(gThreshold, &m_threshold);
    }

    Material *downSample = Engine::loadResource<Material>(".embedded/Downsample.shader");
    if(downSample) {
        m_downMaterial = downSample->createInstance();
    }

    m_inputs.push_back("In");

    m_outputs.push_back(std::make_pair("Result", nullptr));
}

Bloom::~Bloom() {
    delete m_thresholdMaterial;
    delete m_downMaterial;


    m_resultTarget->deleteLater();

    for(auto it : m_bloomPasses) {
        it.blurTempTarget->deleteLater();
        it.downTarget->deleteLater();
        delete it.blurMaterialV;
        delete it.blurMaterialH;

        it.downTexture->deleteLater();
        it.blurTempTexture->deleteLater();
    }
}

void Bloom::analyze(World *world) {
    A_UNUSED(world);
    float threshold = PostProcessSettings::defaultValue(gBloomThreshold).toFloat();
    float intensity = PostProcessSettings::defaultValue(gBloomIntensity).toFloat();
    float dirtIntensity = PostProcessSettings::defaultValue(gBloomDirtIntensity).toFloat();
    Texture *texture = nullptr;

    for(auto pool : m_context->culledPostEffectSettings()) {
        Variant thresholdValue = pool.first->readValue(gBloomThreshold);
        if(thresholdValue.isValid()) {
            threshold = MIX(threshold, thresholdValue.toFloat(), pool.second);
        }
        Variant intensityValue = pool.first->readValue(gBloomIntensity);
        if(intensityValue.isValid()) {
            intensity = MIX(intensity, intensityValue.toFloat(), pool.second);
        }
        Variant dirtIntensityValue = pool.first->readValue(gBloomDirtIntensity);
        if(dirtIntensityValue.isValid()) {
            dirtIntensity = MIX(dirtIntensity, dirtIntensityValue.toFloat(), pool.second);
        }

        texture = pool.first->readValue(gBloomDirt).value<Texture *>();
    }

    if(m_threshold != threshold) {
        m_threshold = threshold;
        m_thresholdMaterial->setFloat(gThreshold, &m_threshold);
    }

    if(m_intensity != intensity) {
        m_intensity = intensity;
        for(auto it : m_bloomPasses) {
            it.blurMaterialV->setFloat(gIntensity, &m_intensity);
            it.blurMaterialH->setFloat(gIntensity, &m_intensity);
        }
    }
    if(m_dirtIntensity != dirtIntensity) {
        m_dirtIntensity = dirtIntensity;
        for(auto it : m_bloomPasses) {
            it.blurMaterialV->setFloat(gDirtIntensity, &m_dirtIntensity);
            it.blurMaterialH->setFloat(gDirtIntensity, &m_dirtIntensity);
        }
    }
    if(m_dirtTexture != texture) {
        for(auto it : m_bloomPasses) {
            it.blurMaterialV->setTexture("dirtMap", texture);
            it.blurMaterialH->setTexture("dirtMap", texture);
        }
    }
}

void Bloom::exec() {
    if(m_mipLevels == 0) {
        return;
    }

    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("Bloom");

    buffer->setViewport(0, 0, m_width, m_height);
    buffer->setRenderTarget(m_bloomPasses[0].downTarget);

    buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_thresholdMaterial);

    for(uint8_t i = 1; i < m_mipLevels; i++) {
        buffer->setViewport(0, 0, (m_width >> i), (m_height >> i));
        buffer->setRenderTarget(m_bloomPasses[i].downTarget);
        m_downMaterial->setTexture(gRgbMap, m_bloomPasses[i-1].downTexture);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_downMaterial);
    }

    Vector2 direction;
    for(int32_t i = m_mipLevels-1; i >= 1; i--) {
        buffer->setViewport(0, 0, (m_width >> i), (m_height >> i));
        buffer->setRenderTarget(m_bloomPasses[i].blurTempTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_bloomPasses[i].blurMaterialH);

        buffer->setViewport(0, 0, m_width, m_height);
        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_bloomPasses[i].blurMaterialV);
    }

    buffer->endDebugMarker();
}

void Bloom::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        Vector2 size(1.0f / (float)width, 1.0f / (float)height);

        m_mipLevels = std::log2(std::min(width, height));
        if(m_bloomPasses.size() < m_mipLevels) {
            m_bloomPasses.resize(m_mipLevels);
        }

        Vector2 directionH(1.0f, 0.0f);
        Vector2 directionV(0.0f, 1.0f);

        for(uint8_t i = 0; i < m_mipLevels; i++) {
            if(m_bloomPasses[i].downTexture == nullptr) {
                m_bloomPasses[i].downTexture = Engine::objectCreate<Texture>("downSampleTexture");
                m_bloomPasses[i].downTexture->setFormat(Texture::RGBA16Float);
                m_bloomPasses[i].downTexture->setFiltering(Texture::Bilinear);
                m_bloomPasses[i].downTexture->setFlags(Texture::Render);
            }
            m_bloomPasses[i].downTexture->resize((width >> i), (height >> i));

            if(m_bloomPasses[i].blurTempTexture == nullptr) {
                m_bloomPasses[i].blurTempTexture = Engine::objectCreate<Texture>("blurTempTexture");
                m_bloomPasses[i].blurTempTexture->setFormat(Texture::RGBA16Float);
                m_bloomPasses[i].blurTempTexture->setFiltering(Texture::Bilinear);
                m_bloomPasses[i].blurTempTexture->setFlags(Texture::Render);
            }
            m_bloomPasses[i].blurTempTexture->resize((width >> i), (height >> i));

            if(m_bloomPasses[i].downTarget == nullptr) {
                m_bloomPasses[i].downTarget = Engine::objectCreate<RenderTarget>("downSampleTarget");

                m_bloomPasses[i].downTarget->setColorAttachment(0, m_bloomPasses[i].downTexture);
            }

            if(m_bloomPasses[i].blurTempTarget == nullptr) {
                m_bloomPasses[i].blurTempTarget = Engine::objectCreate<RenderTarget>("blurTempTarget");

                m_bloomPasses[i].blurTempTarget->setColorAttachment(0, m_bloomPasses[i].blurTempTexture);
                m_bloomPasses[i].blurTempTarget->setFlags(RenderTarget::ClearColor);
            }

            float blurSize = (1 << i);

            float radius = MAX((width >> i), 1) * blurSize * 2.0f * 0.01f;
            m_bloomPasses[i].steps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

            generateKernel(radius, m_bloomPasses[i].steps, m_bloomPasses[i].blurPoints);

            if(m_bloomPasses[i].blurMaterialV == nullptr) {
                Material *blur = Engine::loadResource<Material>(".embedded/Blur.shader");
                if(blur) {
                    m_bloomPasses[i].blurMaterialV = blur->createInstance();
                    m_bloomPasses[i].blurMaterialV->setFloat(gIntensity, &m_intensity);
                    m_bloomPasses[i].blurMaterialV->setFloat(gDirtIntensity, &m_dirtIntensity);

                    m_bloomPasses[i].blurMaterialH = blur->createInstance();
                    m_bloomPasses[i].blurMaterialH->setFloat(gIntensity, &m_intensity);
                    m_bloomPasses[i].blurMaterialH->setFloat(gDirtIntensity, &m_dirtIntensity);
                }
            }

            m_bloomPasses[i].blurMaterialH->setVector2(gSize, &size);
            m_bloomPasses[i].blurMaterialH->setInteger(gSteps, &m_bloomPasses[i].steps);
            m_bloomPasses[i].blurMaterialH->setFloat(gCurve, m_bloomPasses[i].blurPoints);
            m_bloomPasses[i].blurMaterialH->setVector2(gDirection, &directionH);
            m_bloomPasses[i].blurMaterialH->setTexture(gRgbMap, m_bloomPasses[i].downTexture);

            m_bloomPasses[i].blurMaterialV->setVector2(gSize, &size);
            m_bloomPasses[i].blurMaterialV->setInteger(gSteps, &m_bloomPasses[i].steps);
            m_bloomPasses[i].blurMaterialV->setFloat(gCurve, m_bloomPasses[i].blurPoints);
            m_bloomPasses[i].blurMaterialV->setVector2(gDirection, &directionV);
            m_bloomPasses[i].blurMaterialV->setTexture(gRgbMap, m_bloomPasses[i].blurTempTexture);
        }
    }

    PipelineTask::resize(width, height);
}

void Bloom::setInput(int index, Texture *source) {
    if(index == 0) {
        m_downMaterial->setTexture(gRgbMap, source);

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
