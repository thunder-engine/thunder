#include "postprocess/antialiasing.h"

#include "engine.h"

#include "pipelinecontext.h"
#include "material.h"

#include "resources/rendertarget.h"

namespace {
    const char *antialiasing("graphics.antialiasing");
};

#include "commandbuffer.h"

AntiAliasing::AntiAliasing(PipelineContext *context) :
        RenderPass(context) {

    Material *material = Engine::loadResource<Material>(".embedded/SSAA.shader");
    if(material) {
        m_material = material->createInstance();
    }

    m_resultTexture = Engine::objectCreate<Texture>();
    m_resultTexture->setFormat(Texture::R11G11B10Float);

    m_resultTarget = Engine::objectCreate<RenderTarget>();
    m_resultTarget->setColorAttachment(0, m_resultTexture);

    Engine::setValue(antialiasing, true);
}

Texture *AntiAliasing::draw(Texture *source, PipelineContext *context) {
    if(m_enabled && m_material) {
        m_material->setTexture("rgbMap", source);

        CommandBuffer *buffer = context->buffer();

        buffer->setRenderTarget(m_resultTarget);
        buffer->drawMesh(Matrix4(), PipelineContext::defaultPlane(), 0, CommandBuffer::UI, m_material);

        return m_resultTexture;
    }

    return source;
}

void AntiAliasing::resize(int32_t width, int32_t height) {
    m_resultTexture->setWidth(width);
    m_resultTexture->setHeight(height);
}

uint32_t AntiAliasing::layer() const {
    return CommandBuffer::UI;
}

const char *AntiAliasing::name() const {
    return "AntiAliasing";
}

