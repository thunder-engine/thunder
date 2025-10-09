#include "resources/rendertargetmt.h"

#include "resources/texturemt.h"

#include "commandbuffermt.h"

RenderTargetMt::RenderTargetMt() :
        m_descriptor(nullptr),
        m_native(false) {

}

MTL::RenderPassDescriptor *RenderTargetMt::nativeHandle() {
    switch(state()) {
        case ToBeUpdated: {
            if(m_descriptor != nullptr) {
                m_descriptor->release();
            }

            m_descriptor = MTL::RenderPassDescriptor::alloc()->init();

            setState(Ready);
        } break;
        case Unloading: {
            if(m_descriptor != nullptr) {
                m_descriptor->release();
                m_descriptor = nullptr;
            }

            setState(ToBeDeleted);
            return nullptr;
        }
        default: break;
    }

    for(uint32_t i = 0; i < colorAttachmentCount(); i++) {
        TextureMt *color = static_cast<TextureMt *>(colorAttachment(i));
        if(color) {
            MTL::Texture *handle = color->nativeHandle();

            MTL::RenderPassColorAttachmentDescriptor *desc = MTL::RenderPassColorAttachmentDescriptor::alloc()->init();
            desc->setLevel(m_currentLevel);
            desc->setTexture(handle);
            desc->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 0.0));
            desc->setLoadAction((clearFlags() & ClearColor) ? MTL::LoadActionClear : MTL::LoadActionLoad);
            desc->setStoreAction(MTL::StoreActionStore);

            m_descriptor->colorAttachments()->setObject(desc, i);
        }
    }

    TextureMt *depth = static_cast<TextureMt *>(depthAttachment());
    if(depth) {
        MTL::Texture *handle = depth->nativeHandle();

        MTL::RenderPassDepthAttachmentDescriptor *desc = MTL::RenderPassDepthAttachmentDescriptor::alloc()->init();
        desc->setLevel(m_currentLevel);
        desc->setTexture(handle);
        desc->setLoadAction((clearFlags() & ClearDepth) ? MTL::LoadActionClear : MTL::LoadActionLoad);
        desc->setStoreAction(MTL::StoreActionStore);

        m_descriptor->setDepthAttachment(desc);
    }

    return m_descriptor;
}

void RenderTargetMt::setNativeHandle(MTL::RenderPassDescriptor *descriptor) {
    if(m_descriptor != descriptor) {
        m_descriptor = descriptor;
        m_native = true;

        setState(Ready);
    }
}

void RenderTargetMt::setLevel(uint32_t level) {
    m_currentLevel = level;
    if(m_descriptor) {
        m_descriptor->colorAttachments()->object(0)->setLevel(m_currentLevel);
    }
}

bool RenderTargetMt::isNative() const {
    return m_native;
}
