#include "pipelinepasses/gbuffer.h"

#include "components/camera.h"

#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

#define GBUFFER "gBuffer"
#define RADIANCE_MAP "radianceMap"

GBuffer::GBuffer() :
    m_gbuffer(Engine::objectCreate<RenderTarget>(GBUFFER)),
    m_depth(Engine::objectCreate<Texture>(DEPTH_MAP)),
    m_emissive(Engine::objectCreate<Texture>(G_EMISSIVE)),
    m_normals(Engine::objectCreate<Texture>(G_NORMALS)),
    m_diffuse(Engine::objectCreate<Texture>(G_DIFFUSE)),
    m_params(Engine::objectCreate<Texture>(G_PARAMS)),
    m_radiance(Engine::objectCreate<Texture>(RADIANCE_MAP)) {

    m_depth->setFormat(Texture::Depth);
    m_depth->setDepthBits(24);
    m_depth->setWidth(2);
    m_depth->setHeight(2);

    m_emissive->setFormat(Texture::R11G11B10Float);
    m_normals->setFormat(Texture::RGB10A2);
    m_diffuse->setFormat(Texture::RGBA8);
    m_params->setFormat(Texture::RGBA8);

    m_radiance->setFormat(Texture::RGBA8);
    m_radiance->resize(2, 2);
    auto &surface = m_radiance->surface(0);

    uint32_t v = 0x00352400;
    uint32_t *dst = reinterpret_cast<uint32_t *>(surface[0].data());
    for(uint8_t i = 0; i < 4; i++) {
        *dst = v;
        dst++;
    }

    m_gbuffer->setColorAttachment(Emissive, m_emissive);
    m_gbuffer->setColorAttachment(Normals, m_normals);
    m_gbuffer->setColorAttachment(Diffuse, m_diffuse);
    m_gbuffer->setColorAttachment(Params, m_params);
    m_gbuffer->setDepthAttachment(m_depth);
}

Texture *GBuffer::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();

    buffer->setGlobalTexture(RADIANCE_MAP, m_radiance);

    buffer->setViewport(0, 0, m_emissive->width(), m_emissive->height());
    context->cameraReset();

    buffer->setRenderTarget(m_gbuffer);
    buffer->clearRenderTarget(true, context->currentCamera()->color());

    context->drawRenderers(CommandBuffer::DEFAULT, context->culledComponents());

    return m_emissive;
}

void GBuffer::resize(int32_t width, int32_t height) {
    m_depth->setWidth(width);
    m_depth->setHeight(height);

    m_emissive->setWidth(width);
    m_emissive->setHeight(height);

    m_normals->setWidth(width);
    m_normals->setHeight(height);

    m_diffuse->setWidth(width);
    m_diffuse->setHeight(height);

    m_params->setWidth(width);
    m_params->setHeight(height);
}

uint32_t GBuffer::layer() const {
    return CommandBuffer::DEFAULT;
}

uint32_t GBuffer::outputCount() const {
    return OutputsNumber;
}

Texture *GBuffer::output(uint32_t index) {
    switch(index) {
        case Emissive: return m_emissive;
        case Normals: return m_normals;
        case Diffuse: return m_diffuse;
        case Params: return m_params;
        case Depth: return m_depth;
        default: break;
    }
    return nullptr;
}
