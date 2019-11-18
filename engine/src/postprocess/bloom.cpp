#include "postprocess/bloom.h"

#include "engine.h"

#include "pipeline.h"
#include "rendertexture.h"
#include "material.h"

#include "commandbuffer.h"

#include <cmath>
#include <cstring>

Bloom::Bloom() :
        m_Threshold(1.0f) {
    Material *material = Engine::loadResource<Material>(".embedded/Downsample.mtl");
    if(material) {
        m_pMaterial = material->createInstance();
        m_pMaterial->setFloat("threshold", &m_Threshold);
    }

    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        RenderTexture *t = Engine::objectCreate<RenderTexture>();
        t->setTarget(Texture::R11G11B10Float);

        m_BloomPasses[i].m_pDownTexture = t;
    }

    m_BloomPasses[0].m_BlurSize = Vector3(1.0f,  0.0f,  4.0f);
    m_BloomPasses[1].m_BlurSize = Vector3(4.0f,  0.0f,  8.0f);
    m_BloomPasses[2].m_BlurSize = Vector3(16.0f, 0.0f, 16.0f);
    m_BloomPasses[3].m_BlurSize = Vector3(32.0f, 0.0f, 32.0f);
    m_BloomPasses[4].m_BlurSize = Vector3(64.0f, 0.0f, 64.0f);
}

RenderTexture *Bloom::draw(RenderTexture *source, ICommandBuffer &buffer) {
    if(m_pMaterial) {
        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            m_pMaterial->setTexture("rgbMap", (i == 0) ? source : m_BloomPasses[i - 1].m_pDownTexture);
            buffer.setViewport(0, 0, m_BloomPasses[i].m_pDownTexture->width(), m_BloomPasses[i].m_pDownTexture->height());
            buffer.setRenderTarget({m_BloomPasses[i].m_pDownTexture});
            buffer.drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pMaterial);
        }
        buffer.setViewport(0, 0, source->width(), source->height());

        Blur *blur = PostProcessor::blur();
        for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
            blur->setParameters(Vector2(1.0f / m_BloomPasses[i].m_pDownTexture->width(), 1.0f / m_BloomPasses[i].m_pDownTexture->height()),
                                m_BloomPasses[i].m_BlurSteps, m_BloomPasses[i].m_BlurPoints);
            blur->draw(buffer, m_BloomPasses[i].m_pDownTexture, source);
        }
    }

    return source;
}

void Bloom::resize(int32_t width, int32_t height) {
    PostProcessor::resize(width, height);

    uint8_t div = 2;
    for(uint8_t i = 0; i < BLOOM_PASSES; i++) {
        int32_t size = width / div;
        float radius = size * (m_BloomPasses[i].m_BlurSize.x * 1.0f) * 2 * 0.01f;

        m_BloomPasses[i].m_pDownTexture->resize(size, height / div);
        m_BloomPasses[i].m_BlurSteps = CLAMP(static_cast<int32_t>(radius), 0, MAX_SAMPLES);

        memset(m_BloomPasses[i].m_BlurPoints, 0, sizeof(float) * MAX_SAMPLES);

        Blur::generateKernel(radius, m_BloomPasses[i].m_BlurSteps, m_BloomPasses[i].m_BlurPoints);

        div *= 2;
    }
}
