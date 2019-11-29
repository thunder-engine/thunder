#include "postprocess/ambientocclusion.h"

#include "engine.h"

#include "components/postprocesssettings.h"

#include "resources/pipeline.h"
#include "resources/material.h"
#include "resources/rendertexture.h"

#include "filters/blur.h"

#include "commandbuffer.h"

#include "amath.h"

#include <cstring>

#define BLUR_STEPS 4

#define SSAO_MAP    "uni.ssaoMap"

AmbientOcclusion::AmbientOcclusion() :
        m_Radius(0.2f),
        m_Bias(0.025f),
        m_Power(2.0f) {

    for(int32_t i = 0; i < KERNEL_SIZE; i++) {
        m_SamplesKernel[i].x = RANGE( 0.0f, 1.0f) * 2.0f - 1.0f;
        m_SamplesKernel[i].y = RANGE( 0.0f, 1.0f) * 2.0f - 1.0f;
        m_SamplesKernel[i].z = RANGE( 0.0f, 1.0f);

        m_SamplesKernel[i].normalize();
        m_SamplesKernel[i] *= RANGE( 0.0f, 1.0f);

        float scale = static_cast<float>(i) / KERNEL_SIZE;
        scale = MIX(0.1f, 1.0f, scale * scale);
        m_SamplesKernel[i] *= scale;
    }

    m_pNoise = Engine::objectCreate<Texture>();
    m_pNoise->setFormat(Texture::RGB16Float);
    m_pNoise->setWrap(Texture::Repeat);
    m_pNoise->setFiltering(Texture::None);
    m_pNoise->resize(4, 4);

    Texture::Surface &s = m_pNoise->surface(0);

    Vector3 *ptr = reinterpret_cast<Vector3 *>(s[0]);
    for(int32_t i = 0; i < KERNEL_SIZE; i++) {
        ptr[i].x = RANGE( 0.0f, 1.0f) * 2.0f - 1.0f;
        ptr[i].y = RANGE( 0.0f, 1.0f) * 2.0f - 1.0f;
        ptr[i].z = 0.0f;

        ptr[i].normalize();
    }
    {
        Material *material = Engine::loadResource<Material>(".embedded/AmbientOcclusion.mtl");
        if(material) {
            material->setTexture("noiseMap", m_pNoise);

            m_pMaterial = material->createInstance();
            m_pMaterial->setVector3("samplesKernel", m_SamplesKernel, KERNEL_SIZE);

            m_pMaterial->setFloat("radius", &m_Radius);
            m_pMaterial->setFloat("bias", &m_Bias);
            m_pMaterial->setFloat("power", &m_Power);
        }
    }
    {
        Material *mtl = Engine::loadResource<Material>(".embedded/CombineOcclusion.mtl");
        if(mtl) {
            m_pOcclusion = mtl->createInstance();
        }
    }

    m_pSSAO = Engine::objectCreate<RenderTexture>();
    m_pSSAO->setTarget(Texture::R8);

    m_pResultTexture = Engine::objectCreate<RenderTexture>();
    m_pResultTexture->setTarget(Texture::R8);
}

AmbientOcclusion::~AmbientOcclusion() {
    m_pNoise->deleteLater();
}

RenderTexture *AmbientOcclusion::draw(RenderTexture *source, ICommandBuffer &buffer) {
    if(m_Enabled) {
        if(m_pMaterial) {
            buffer.setViewport(0, 0, m_pSSAO->width(), m_pSSAO->height());
            buffer.setGlobalValue("camera.screen", Vector4(1.0f / m_pSSAO->width(), 1.0f / m_pSSAO->height(),
                                                           m_pSSAO->width(), m_pSSAO->height()));
            buffer.setRenderTarget({m_pSSAO});
            buffer.drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pMaterial);

            buffer.setGlobalTexture(SSAO_MAP, m_pSSAO);

            buffer.setViewport(0, 0, source->width(), source->height());
            buffer.setGlobalValue("camera.screen", Vector4(1.0f / source->width(), 1.0f / source->height(),
                                                           source->width(), source->height()));

            //Blur *blur = PostProcessor::blur();
            //blur->setParameters(Vector2(1.0f / m_pResultTexture->width(), 1.0f / m_pResultTexture->height()), BLUR_STEPS, m_BlurSamplesKernel);
            //blur->draw(buffer, m_pSSAO, m_pResultTexture);
        }

        if(m_pOcclusion) {
            buffer.setRenderTarget({source});
            buffer.drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pOcclusion);
        }

        return m_pSSAO;
    }
    return source;
}

void AmbientOcclusion::resize(int32_t width, int32_t height) {
    m_pSSAO->resize(width / 2, height / 2);
    m_pResultTexture->resize(width, height);

    float radius = width * 0.01f;
    memset(m_BlurSamplesKernel, 0, sizeof(float) * BLUR_STEPS);
    Blur::generateKernel(radius, BLUR_STEPS, m_BlurSamplesKernel);
}

void AmbientOcclusion::setSettings(const PostProcessSettings &settings) {
    m_Enabled = settings.ambientOcclusionEnabled();
    m_Radius = settings.ambientOcclusionRadius();
    m_Bias = settings.ambientOcclusionBias();
    m_Power = settings.ambientOcclusionPower();
}
