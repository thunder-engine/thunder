/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
            desc->setLoadAction((flags() & ClearColor) ? MTL::LoadActionClear : MTL::LoadActionLoad);
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
        desc->setLoadAction((flags() & ClearDepth) ? MTL::LoadActionClear : MTL::LoadActionLoad);
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

MTL::Buffer *RenderTargetMt::globalBuffer(size_t currentFrame) {
    size_t index = currentFrame;
    if(flags() & Atlas) {
        index += tileIndex() * WrapperMt::framesInFlight();
    }
    return m_global[index];
}

void RenderTargetMt::updateGlobalMemory(size_t currentFrame, const Global &global) {
    size_t swapChainCount = WrapperMt::framesInFlight();
    size_t index = currentFrame;
    if(flags() & Atlas) {
        index += tileIndex() * swapChainCount;
        swapChainCount *= 32;
    }

    if(m_global.empty()) {
        m_global.resize(swapChainCount);
        for(size_t i = 0; i < swapChainCount; i++) {
            m_global[i] = WrapperMt::device()->newBuffer(sizeof(Global), MTL::ResourceStorageModeShared);
        }
    }
    uint8_t *ptr = reinterpret_cast<uint8_t *>(m_global[index]->contents());
    if(ptr) {
        memcpy(ptr, &global, sizeof(Global));
    }
}
