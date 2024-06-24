#ifndef SURFACEVK_H
#define SURFACEVK_H

#include <vulkan/vulkan.h>

#include <vector>

class TextureVk;
class RenderTarget;

class SurfaceVk {
public:
    void selectFormats();
    void recreateSwapChain();
    void releaseSwapChain();

    bool beginFrame(uint32_t width, uint32_t height);
    void endFrame();

    VkCommandBuffer currentCmdBuffer() const;

    void setupCurrentTarget(RenderTarget *target) const;

private:
    std::vector<TextureVk *> m_images;

    std::vector<VkFramebuffer> m_frameBuffers;

    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    TextureVk *m_depth = nullptr;

    uint32_t m_currentFrame = 0;

    uint32_t m_imageIndex = 0;

public:
    VkSurfaceKHR m_nativeSurface = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    static uint32_t swapChainImageCount;

};

#endif // SURFACEVK_H
