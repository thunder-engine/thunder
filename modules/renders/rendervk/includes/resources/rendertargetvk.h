#ifndef RENDERTARGETVK_H
#define RENDERTARGETVK_H

#include <resources/rendertarget.h>

#include <vulkan/vulkan.h>

class RenderTargetVk : public RenderTarget {
    A_OVERRIDE(RenderTargetVk, RenderTarget, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetVk();

    void bindTarget(VkCommandBuffer &buffer, uint32_t level);

private:
    bool updateBuffer(VkCommandBuffer &buffer, uint32_t level);
    void destroyBuffer();

    VkRenderPass m_renderPass;
    VkFramebuffer m_frameBuffer;

};

#endif // RENDERTARGETVK_H
