#include "pipelinetasks/translucent.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/rendertarget.h"
#include "resources/mesh.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

Translucent::Translucent() :
        m_translucentPass(Engine::objectCreate<RenderTarget>("translucentPass")) {

    m_inputs.push_back("In");
    m_inputs.push_back("Depth");

    m_outputs.push_back(std::make_pair("Result", nullptr));

}

void Translucent::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();
    buffer->beginDebugMarker("Translucent Pass");

    buffer->setRenderTarget(m_translucentPass);

    // Transparent pass
    context.drawRenderers(context.culledComponents(), CommandBuffer::TRANSLUCENT);

    buffer->endDebugMarker();
}

void Translucent::setInput(int index, Texture *texture) {
    if(texture->depthBits() > 0) {
        m_translucentPass->setDepthAttachment(texture);
    } else {
        m_translucentPass->setColorAttachment(0, texture);

        m_outputs.front().second = texture;
    }
}
