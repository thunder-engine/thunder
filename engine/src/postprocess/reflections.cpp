#include "postprocess/reflections.h"

#include "engine.h"

#include "components/postprocesssettings.h"

#include "resources/pipeline.h"
#include "resources/material.h"
#include "resources/rendertexture.h"

#include "filters/blur.h"

#include "commandbuffer.h"

#include "amath.h"

#define BLUR_STEPS 4

Reflections::Reflections() :
        m_pIblMaterial(nullptr),
        m_pEnvironmentTexture(nullptr) {
    m_pResultTexture = Engine::objectCreate<RenderTexture>();
    m_pResultTexture->setTarget(Texture::RGBA32Float);

    {
        Material *material = Engine::loadResource<Material>(".embedded/LocalReflections.mtl");
        if(material) {
            m_pMaterial = material->createInstance();
        }
    }
    {
        Material *material = Engine::loadResource<Material>(".embedded/IblReflections.mtl");
        if(material) {
            m_pIblMaterial = material->createInstance();
            m_pIblMaterial->setTexture("rgbMap", m_pResultTexture);
            m_pIblMaterial->setTexture("environmentMap", m_pEnvironmentTexture);
        }
    }
}

Reflections::~Reflections() {

}

RenderTexture *Reflections::draw(RenderTexture *source, Pipeline *pipeline) {
    if(m_Enabled) {
        ICommandBuffer *buffer = pipeline->buffer();
        if(m_pMaterial) {
            buffer->setRenderTarget({m_pResultTexture});
            buffer->drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pMaterial);
        }
        if(m_pIblMaterial) {
            buffer->setRenderTarget({source});
            buffer->drawMesh(Matrix4(), m_pMesh, ICommandBuffer::UI, m_pIblMaterial);
        }
    }
    return m_pResultTexture;
}

void Reflections::setSettings(const PostProcessSettings &settings) {
    m_Enabled = settings.reflectionsEnabled();
}

void Reflections::resize(int32_t width, int32_t height) {
    m_pResultTexture->resize(width, height);
}

uint32_t Reflections::layer() const {
    return ICommandBuffer::LIGHT;
}
