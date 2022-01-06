#include "filters/blur.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"

#include "engine.h"
#include "commandbuffer.h"

#define OVERRIDE "rgbMap"

Blur::Blur() :
        m_pBlurMaterial(nullptr) {

    m_pMesh = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    Material *material = Engine::loadResource<Material>(".embedded/Blur.shader");
    if(material) {
        m_pBlurMaterial = material->createInstance();
    }

    m_tempTexture = Engine::objectCreate<Texture>();
    m_tempTexture->setFormat(Texture::R11G11B10Float);

    m_tempTarget = Engine::objectCreate<RenderTarget>();
    m_tempTarget->setColorAttachment(0, m_tempTexture);
}

void Blur::draw(CommandBuffer &buffer, Texture *source, RenderTarget *target) {
    if(m_pBlurMaterial) {
        Texture *t = target->colorAttachment(0);
        m_tempTexture->setWidth(t->width());
        m_tempTexture->setHeight(t->height());

        m_direction.x = 1.0f;
        m_direction.y = 0.0f;
        m_pBlurMaterial->setVector2("uni.direction", &m_direction);

        m_pBlurMaterial->setTexture(OVERRIDE, source);

        buffer.setRenderTarget(m_tempTarget);
        buffer.clearRenderTarget();
        buffer.drawMesh(Matrix4(), m_pMesh, 0, CommandBuffer::UI, m_pBlurMaterial);

        m_direction.x = 0.0f;
        m_direction.y = 1.0f;
        m_pBlurMaterial->setVector2("uni.direction", &m_direction);

        m_pBlurMaterial->setTexture(OVERRIDE, m_tempTexture);

        buffer.setRenderTarget(target);
        buffer.drawMesh(Matrix4(), m_pMesh, 0, CommandBuffer::UI, m_pBlurMaterial);
    }
}

void Blur::setParameters(const Vector2 &size, int32_t steps, const float *points) {
    if(m_pBlurMaterial) {
        m_pBlurMaterial->setInteger("uni.steps", &steps);
        m_pBlurMaterial->setFloat("uni.curve", points);
        m_pBlurMaterial->setVector2("uni.size", &size);
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

