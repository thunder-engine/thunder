#include "pipelinetasks/translucent.h"

#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

Translucent::Translucent() :
        m_translucentPass(Engine::objectCreate<RenderTarget>("translucentPass")) {

    setName("Translucent");

    m_inputs.push_back("In");
    m_inputs.push_back("Depth");

    m_outputs.push_back(std::make_pair("Result", nullptr));
}

void Translucent::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();

    buffer->beginDebugMarker("TranslucentPass");

    buffer->setRenderTarget(m_translucentPass);

    context.drawRenderers(context.culledComponents(), CommandBuffer::TRANSLUCENT);

    buffer->endDebugMarker();
}

void Translucent::setInput(int index, Texture *texture) {
    switch(index) {
        case 0: {
            m_translucentPass->setColorAttachment(0, texture);
            m_outputs.front().second = texture;
        } break;
        case 1: {
            m_translucentPass->setDepthAttachment(texture);
        } break;
        default: break;
    }
}
