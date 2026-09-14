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
#include "resources/computebuffervk.h"

#include <cstring>

#include "wrappervk.h"

ComputeBufferVk::ComputeBufferVk() :
        m_buffer(VK_NULL_HANDLE),
        m_memoryBuffer(VK_NULL_HANDLE),
        m_bufferSize(0) {

}

void ComputeBufferVk::bind(VkCommandBuffer buffer) {
    switch(state()) {
        case ToBeUpdated: {
            updateGpu();

            setState(Ready);
        } break;
        case Unloading: {
            destroyGpu();

            switchState(ToBeDeleted);
            return;
        }
        default: break;
    }
}

void ComputeBufferVk::switchState(State state) {
    switch(state) {
        case Unloading: {
            destroyGpu();
        } break;
        default: ComputeBuffer::switchState(state); break;
    }
}

void ComputeBufferVk::updateGpu() {
    VkDevice device = WrapperVk::device();

    auto v = data();
    if(!v.empty()) {
        size_t size = sizeof(v[0]) * v.size();

        if(size > m_bufferSize) {
            WrapperVk::destroyBuffer(m_buffer);

            m_buffer = WrapperVk::createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
            m_memoryBuffer = WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer);
        }

        m_bufferSize = size;

        void *dst = nullptr;
        vkMapMemory(device, m_memoryBuffer, 0, m_bufferSize, 0, &dst);
            memcpy(dst, v.data(), m_bufferSize);
        vkUnmapMemory(device, m_memoryBuffer);
    }
}

void ComputeBufferVk::destroyGpu() {
    WrapperVk::destroyBuffer(m_buffer);
    m_buffer = nullptr;

    WrapperVk::freeMemory(m_memoryBuffer);

    m_bufferSize = 0;
}
