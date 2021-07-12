#include "postprocess/reflections.h"

#include "engine.h"

#include "components/postprocesssettings.h"

#include "resources/pipeline.h"
#include "resources/material.h"
#include "resources/rendertarget.h"

#include "filters/blur.h"

#include "commandbuffer.h"

#include "amath.h"

#define BLUR_STEPS 4

Reflections::Reflections() :
        m_iblMaterial(nullptr),
        m_environmentTexture(nullptr) {

    m_resultTexture = Engine::objectCreate<Texture>();
    m_resultTexture->setFormat(Texture::RGBA32Float);

    m_resultTarget->setColorAttachment(0, m_resultTexture);

    m_sslrTexture = Engine::objectCreate<Texture>();
    m_sslrTexture->setFormat(Texture::RGBA32Float);

    m_sslrTarget = Engine::objectCreate<RenderTarget>();
    m_sslrTarget->setColorAttachment(0, m_sslrTexture);

    {
        Material *material = Engine::loadResource<Material>(".embedded/LocalReflections.mtl");
        if(material) {
            m_material = material->createInstance();
        }
    }
    {
        Material *material = Engine::loadResource<Material>(".embedded/IblReflections.mtl");
        if(material) {
            m_iblMaterial = material->createInstance();
            m_iblMaterial->setTexture("rgbMap", m_sslrTexture);
            m_iblMaterial->setTexture("environmentMap", m_environmentTexture);
        }
    }
}

Reflections::~Reflections() {

}

Texture *Reflections::draw(Texture *source, Pipeline *pipeline) {
    if(m_enabled) {
        ICommandBuffer *buffer = pipeline->buffer();
        if(m_material) { // sslr step
            buffer->setRenderTarget(m_sslrTarget);
            buffer->drawMesh(Matrix4(), m_mesh, ICommandBuffer::UI, m_material);
        }

        if(m_iblMaterial) { // combine step
            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(Matrix4(), m_mesh, ICommandBuffer::UI, m_iblMaterial);
        }

        return m_resultTexture;
    }

    return source;
}

void Reflections::setSettings(const PostProcessSettings &settings) {
    m_enabled = settings.reflectionsEnabled();
}

void Reflections::resize(int32_t width, int32_t height) {
    m_resultTexture->setWidth(width);
    m_resultTexture->setHeight(height);
}

uint32_t Reflections::layer() const {
    return ICommandBuffer::LIGHT;
}
