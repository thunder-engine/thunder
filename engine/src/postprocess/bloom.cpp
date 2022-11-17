#include "postprocess/bloom.h"

#include "engine.h"

#include "resources/rendertarget.h"
#include "resources/material.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "components/private/postprocessorsettings.h"

#include <cmath>
#include <cstring>

namespace {
    const char *bloom("graphics.bloom");

    const char *bloomThreshold("bloom/Threshold");
};


Bloom::Bloom(PipelineContext *context) :
        RenderPass(context),
        m_threshold(1.0f),
        m_width(0),
        m_height(0) {

    Material *material = Engine::loadResource<Material>(".embedded/Downsample.shader");
    if(material) {
        m_material = material->createInstance();
        m_material->setFloat("uni.threshold", &m_threshold);
    }

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        Texture *t = Engine::objectCreate<Texture>();
        t->setFormat(Texture::R11G11B10Float);

        m_bloomPasses[i].m_downTexture = t;
    }

    m_resultTarget = Engine::objectCreate<RenderTarget>();

    Engine::setValue(bloom, true);

    m_bloomPasses[0].m_blurSize = Vector3(1.0f,  0.0f,  4.0f);
    m_bloomPasses[1].m_blurSize = Vector3(4.0f,  0.0f,  8.0f);
    m_bloomPasses[2].m_blurSize = Vector3(16.0f, 0.0f, 16.0f);
    m_bloomPasses[3].m_blurSize = Vector3(32.0f, 0.0f, 32.0f);
    m_bloomPasses[4].m_blurSize = Vector3(64.0f, 0.0f, 64.0f);

    PostProcessSettings::registerSetting(bloomThreshold, m_threshold);
}

Texture *Bloom::draw(Texture *source, PipelineContext *context) {
    if(m_material) {
        CommandBuffer *buffer = context->buffer();

        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            m_material->setTexture("rgbMap", (i == 0) ? source : m_bloomPasses[i - 1].m_downTexture);

            buffer->setViewport(0, 0, m_bloomPasses[i].m_downTexture->width(), m_bloomPasses[i].m_downTexture->height());

            m_resultTarget->setColorAttachment(0, m_bloomPasses[i].m_downTexture);
            buffer->setRenderTarget(m_resultTarget);

            buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_material);
        }
        buffer->setViewport(0, 0, source->width(), source->height());

        m_resultTarget->setColorAttachment(0, source);

        static Blur blur;
        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            blur.setParameters(Vector2(1.0f / m_bloomPasses[i].m_downTexture->width(), 1.0f / m_bloomPasses[i].m_downTexture->height()),
                                m_bloomPasses[i].m_blurSteps, m_bloomPasses[i].m_blurPoints);
            blur.draw(*buffer, m_bloomPasses[i].m_downTexture, m_resultTarget);
        }
    }

    return source;
}

void Bloom::resize(int32_t width, int32_t height) {
    RenderPass::resize(width, height);

    if(m_width != width || m_height != height) {
        m_width = width;
        m_height = height;

        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            int32_t size = (width >> i);
            float radius = size * (m_bloomPasses[i].m_blurSize.x * 1.0f) * 2 * 0.01f;

            m_bloomPasses[i].m_downTexture->setWidth(size);
            m_bloomPasses[i].m_downTexture->setHeight(height >> i);
            m_bloomPasses[i].m_blurSteps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

            memset(m_bloomPasses[i].m_blurPoints, 0, sizeof(float) * MAX_SAMPLES);

            Blur::generateKernel(radius, m_bloomPasses[i].m_blurSteps, m_bloomPasses[i].m_blurPoints);
        }
    }
}

void Bloom::setSettings(const PostProcessSettings &settings) {
    m_threshold = settings.readValue(bloomThreshold).toFloat();
    m_material->setFloat("uni.threshold", &m_threshold);
}

const char *Bloom::name() const {
    return "Bloom";
}
