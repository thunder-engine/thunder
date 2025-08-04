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
            buffer->drawMesh(it.mesh, it.subMesh, Material::Translucent, *it.instance);
            it.instance->resetBatches();
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
    RenderList translucent;

    filterByLayer(m_context->culledRenderables(), translucent, Material::Translucent);

    Vector3 cameraWP(Camera::current()->transform()->worldPosition());

    translucent.sort([cameraWP](const Renderable *left, const Renderable *right) {
        int p1 = left->priority();
        int p2 = right->priority();
        if(p1 == p2) {
            return cameraWP.dot(left->transform()->worldPosition()) < cameraWP.dot(right->transform()->worldPosition());
        }
        return p1 < p2;
    });

    m_translucent.clear();

    filterAndGroup(translucent, m_translucent, 0);
}
