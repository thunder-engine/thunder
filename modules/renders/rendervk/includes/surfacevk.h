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
