#ifndef COMPUTEBUFFERVK_H
#define COMPUTEBUFFERVK_H

#include <vulkan/vulkan.h>

#include <resources/computebuffer.h>

class ComputeBufferVk : public ComputeBuffer {
    A_OVERRIDE(ComputeBufferVk, ComputeBuffer, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    ComputeBufferVk();

    void bind(VkCommandBuffer buffer);

protected:
    void switchState(State state) override;

    void updateGpu();

    void destroyGpu();

public:
    VkBuffer m_buffer;

    VkDeviceMemory m_memoryBuffer;

    size_t m_bufferSize;

};

#endif // COMPUTEBUFFERVK_H
