#include "pipelinepasses/reflections.h"

#include "engine.h"

#include "components/scenegraph.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "filters/blur.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#include "amath.h"

namespace {
    const char *reflections("graphics.reflections");
};

Reflections::Reflections() :
        m_slrMaterial(nullptr),
        m_iblMaterial(nullptr),
        m_slrTexture(Engine::objectCreate<Texture>("ScreenLocalReflection")),
        m_environmentTexture(nullptr),
        m_resultTexture(Engine::objectCreate<Texture>("IblReflection")),
        m_slrTarget(Engine::objectCreate<RenderTarget>()),
        m_resultTarget(Engine::objectCreate<RenderTarget>()) {

    setName("Reflections");

    Engine::setValue(reflections, true);

    m_slrTexture->setFormat(Texture::RGBA32Float);
    m_resultTexture->setFormat(Texture::RGBA32Float);

    m_resultTarget->setColorAttachment(0, m_resultTexture);
    m_slrTarget->setColorAttachment(0, m_slrTexture);

    {
        Material *material = Engine::loadResource<Material>(".embedded/SSLR.shader");
        if(material) {
            m_slrMaterial = material->createInstance();
        }
    }
    {
        Material *material = Engine::loadResource<Material>(".embedded/IblReflections.shader");
        if(material) {
            m_iblMaterial = material->createInstance();
            m_iblMaterial->setTexture("rgbMap", m_slrTexture);
            m_iblMaterial->setTexture("environmentMap", m_environmentTexture);
        }
    }
}

Texture *Reflections::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();
    if(m_slrMaterial) { // sslr step
        buffer->setRenderTarget(m_slrTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_slrMaterial);
    }

    if(m_iblMaterial) { // combine step
        buffer->setRenderTarget(m_resultTarget);
        buffer->clearRenderTarget();
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_iblMaterial);

        //return m_resultTexture;
    }

    return source;
}

void Reflections::resize(int32_t width, int32_t height) {
    m_resultTexture->setWidth(width);
    m_resultTexture->setHeight(height);

    m_slrTexture->setWidth(width);
    m_slrTexture->setHeight(height);
}

uint32_t Reflections::layer() const {
    return CommandBuffer::LIGHT;
}

uint32_t Reflections::outputCount() const {
    return 0;
}

Texture *Reflections::output(uint32_t index) {
    switch(index) {
        case 0: return m_slrTexture;
        case 1: return m_resultTexture;
        default: break;
    }
    return nullptr;
}
