#include "pipelinepasses/translucent.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/rendertarget.h"
#include "resources/mesh.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

Translucent::Translucent() :
    m_translucentPass(Engine::objectCreate<RenderTarget>("translucentPass")) {

}

Texture *Translucent::draw(Texture *source, PipelineContext *context) {
    CommandBuffer *buffer = context->buffer();

    buffer->setRenderTarget(m_translucentPass);

    // Transparent pass
    context->drawRenderers(CommandBuffer::TRANSLUCENT, context->culledComponents());

    return source;
}

void Translucent::setInput(uint32_t index, Texture *texture) {
    if(texture) {
        switch(index) {
        case Emissve: m_translucentPass->setColorAttachment(Emissve, texture); break;
        case Depth: m_translucentPass->setDepthAttachment(texture); break;
        default: break;
        }
    }
}

uint32_t Translucent::layer() const {
    return CommandBuffer::TRANSLUCENT;
}
