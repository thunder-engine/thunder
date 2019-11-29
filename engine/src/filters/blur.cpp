#include "filters/blur.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertexture.h"

#include "engine.h"
#include "commandbuffer.h"

Blur::Blur() :
        m_pBlurMaterial(nullptr),
        m_Steps(1) {

    m_pMesh = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    Material *material = Engine::loadResource<Material>(".embedded/Blur.mtl");
    if(material) {
        m_pBlurMaterial = material->createInstance();
        m_pBlurMaterial->setInteger("steps", &m_Steps);
        m_pBlurMaterial->setFloat("curve", m_Points, MAX_SAMPLES);

        m_pBlurMaterial->setVector2("size", &m_Size);
        m_pBlurMaterial->setVector2("direction", &m_Direction);
    }

    m_Temp = Engine::objectCreate<RenderTexture>();
    m_Temp->setTarget(Texture::R11G11B10Float);
}

void Blur::draw(ICommandBuffer &buffer, RenderTexture *source, RenderTexture *target) {
    if(m_pBlurMaterial) {
        m_Temp->resize(target->width(), target->height());

        m_Direction.x = 1.0f;
        m_Direction.y = 0.0f;

        m_pBlurMaterial->setTexture("rgbMap", source);

        buffer.setRenderTarget({m_Temp});
        buffer.clearRenderTarget();
        buffer.drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pBlurMaterial);

        m_Direction.x = 0.0f;
        m_Direction.y = 1.0f;

        m_pBlurMaterial->setTexture("rgbMap", m_Temp);

        buffer.setRenderTarget({target});
        //buffer.clearRenderTarget(true, Vector4());
        buffer.drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pBlurMaterial);
    }
}

void Blur::setParameters(const Vector2 &size, int32_t steps, float *points) {
    m_Size = size;
    m_Steps = steps;

    for(int32_t i = 0; i < MAX_SAMPLES; i++) {
        m_Points[i] = points[i];
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

