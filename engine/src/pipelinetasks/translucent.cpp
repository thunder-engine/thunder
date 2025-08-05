#include "pipelinetasks/translucent.h"

#include "resources/rendertarget.h"

#include "components/renderable.h"
#include "components/transform.h"
#include "components/camera.h"

#include "commandbuffer.h"

Translucent::Translucent() :
        m_translucentPass(Engine::objectCreate<RenderTarget>("translucentPass")) {

    setName("Translucent");

    m_inputs.push_back("In");
    m_inputs.push_back("Depth");

    m_outputs.push_back(std::make_pair("Result", nullptr));
}

void Translucent::exec() {
    CommandBuffer *buffer = m_context->buffer();

    if(!m_translucent.empty()) {
        buffer->beginDebugMarker("TranslucentPass");

        buffer->setRenderTarget(m_translucentPass);

        for(auto &it : m_translucent) {
            it.instance->setInstanceBuffer(&it.buffer);
            buffer->drawMesh(it.mesh, it.subMesh, Material::Translucent, *it.instance);
            it.instance->setInstanceBuffer(nullptr);
        }

        buffer->endDebugMarker();
    }
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

void Translucent::analyze(World *world) {
    GroupList list;
    filterByLayer(m_context->culledRenderables(), list, Material::Translucent);

    m_translucent.clear();
    group(list, m_translucent);
}
