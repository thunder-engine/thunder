#include "filters/blur.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"

#include "engine.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

#define OVERRIDE "rgbMap"

Blur::Blur() :
        m_blurMaterial(nullptr),
        m_mesh(PipelineContext::defaultPlane()),
        m_tempTexture(Engine::objectCreate<Texture>("blurTempTexture")),
        m_tempTarget(Engine::objectCreate<RenderTarget>("blurTempTarget")) {

    Material *material = Engine::loadResource<Material>(".embedded/Blur.shader");
    if(material) {
        m_blurMaterial = material->createInstance();
    }

    m_tempTexture->setFormat(Texture::R11G11B10Float);
    m_tempTexture->setFiltering(Texture::Bilinear);
    m_tempTexture->setFlags(Texture::Render);

    m_tempTarget->setColorAttachment(0, m_tempTexture);
}

void Blur::draw(CommandBuffer &buffer, Texture *source, RenderTarget *target) {
    if(m_blurMaterial) {
        Texture *t = target->colorAttachment(0);
        m_tempTexture->resize(t->width(), t->height());

        m_direction.x = 1.0f;
        m_direction.y = 0.0f;
        m_blurMaterial->setVector2("direction", &m_direction);

        m_blurMaterial->setTexture(OVERRIDE, source);

        buffer.setRenderTarget(m_tempTarget);
        buffer.clearRenderTarget();
        buffer.drawMesh(Matrix4(), m_mesh, 0, CommandBuffer::UI, m_blurMaterial);

        m_direction.x = 0.0f;
        m_direction.y = 1.0f;
        m_blurMaterial->setVector2("direction", &m_direction);

        m_blurMaterial->setTexture(OVERRIDE, m_tempTexture);

        buffer.setRenderTarget(target);
        buffer.drawMesh(Matrix4(), m_mesh, 0, CommandBuffer::UI, m_blurMaterial);
    }
}

void Blur::setParameters(const Vector2 &size, int32_t steps, const float *points) {
    if(m_blurMaterial) {
        m_blurMaterial->setInteger("steps", &steps);
        m_blurMaterial->setFloat("curve", points);
        m_blurMaterial->setVector2("size", &size);
    }
}

void Blur::generateKernel(float radius, int32_t steps, float *points) {
    float total = 0.0f;
    for(uint8_t p = 0; p < steps; p++) {
        float weight = std::exp(-static_cast<float>(p * p) / (2.0f * radius));
        points[p] = weight;

        total += weight;
    }

    for(uint8_t p = 0; p < steps; p++) {
        points[p] *= 1.0f / total * 0.5f;// 1.0 / (sqrt(2.0 * PI) * sigma;
    }
}

