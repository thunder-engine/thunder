#include "resources/computebuffervk.h"

#include <cstring>

#include "commandbuffervk.h"
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

            WrapperVk::createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_buffer);
            WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_memoryBuffer);
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
