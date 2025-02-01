#include "surfacevk.h"

#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "wrappervk.h"

#include <log.h>

const uint32_t maxFramesInFlight = 2;

static VkSurfaceFormatKHR s_selectedFormat = {};

uint32_t SurfaceVk::swapChainImageCount = 0;

void SurfaceVk::selectFormats() {
    VkPhysicalDevice physicalDevice = WrapperVk::physicalDevice();

    // Color
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_nativeSurface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats;

    if(formatCount != 0) {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_nativeSurface, &formatCount, formats.data());
    }

    for(const auto &format : formats) {
        if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            s_selectedFormat = format;
        }
    }

    WrapperVk::device(m_nativeSurface);
}

void SurfaceVk::recreateSwapChain() {
    /// \todo: select presentation mode

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(WrapperVk::physicalDevice(), m_nativeSurface, &capabilities);

    VkExtent2D bufferSize = capabilities.currentExtent;

    swapChainImageCount = MAX(MIN(capabilities.maxImageCount, maxFramesInFlight), capabilities.minImageCount);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_nativeSurface;
    createInfo.minImageCount = swapChainImageCount;
    createInfo.imageFormat = s_selectedFormat.format;
    createInfo.imageColorSpace = s_selectedFormat.colorSpace;
    createInfo.imageExtent = bufferSize;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = true;
    createInfo.oldSwapchain = m_swapChain;

    // Could be VK_SHARING_MODE_CONCURRENT if queues gfx != presentation
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    VkSwapchainKHR newSwapChain;

    VkDevice device = WrapperVk::device();
    VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapChain);
    if(result != VK_SUCCESS) {
        aError() << "failed to create swap chain!";
    }

    if(m_swapChain) {
        releaseSwapChain();
    }
    m_swapChain = newSwapChain;

    std::vector<VkImage> swapChainImages;

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, swapChainImages.data());

    if(m_images.empty()) {
        for(uint32_t i = 0; i < imageCount; i++) {
            TextureVk *image = new TextureVk;
            image->setFlags(Texture::Render);

            m_images.push_back(image);
        }
    }

    uint32_t index = 0;
    for(auto it : swapChainImages) {
        m_images[index]->resize(bufferSize.width, bufferSize.height);
        m_images[index]->setImage(device, s_selectedFormat.format, it);

        ++index;
    }

    // Depth buffer
    if(m_depth == nullptr) {
        m_depth = new TextureVk;
        m_depth->setFlags(Texture::Render);
        m_depth->setFormat(Texture::Depth);
    }

    VkFormat depthFormat = m_depth->vkFormat();
    m_depth->resize(bufferSize.width, bufferSize.height);
    m_depth->destroyImage(device);
    m_depth->setImage(device, depthFormat, m_depth->createImage(device, depthFormat));

    // Default render pass
    VkAttachmentDescription attachDesc[2];
    attachDesc[0] = {};
    attachDesc[0].format = s_selectedFormat.format;
    attachDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachDesc[1] = {};
    attachDesc[1].format = depthFormat;
    attachDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if(m_renderPass) {
        vkDestroyRenderPass(device, m_renderPass, nullptr);
    }

    result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass);
    if(result != VK_SUCCESS) {
        aError() << "failed to create render pass!";
    }

    // Framebuffers
    m_frameBuffers.resize(swapChainImages.size());

    for(size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageView attachments[] = { m_images[i]->vkView(),
                                      m_depth->vkView() };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_images[i]->width();
        framebufferInfo.height = m_images[i]->height();
        framebufferInfo.layers = 1;

        if(m_frameBuffers[i]) {
            vkDestroyFramebuffer(device, m_frameBuffers[i], nullptr);
        }

        result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_frameBuffers[i]);

        if(result != VK_SUCCESS) {
            aError() << "failed to create framebuffer!";
        }
    }

    if(m_commandPool == VK_NULL_HANDLE) {
        m_commandPool = WrapperVk::commandPool();
    }

    // Command buffers
    if(m_commandBuffers.empty()) {
        m_commandBuffers.resize(swapChainImageCount);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

        result = vkAllocateCommandBuffers(device, &allocInfo, m_commandBuffers.data());

        if(result != VK_SUCCESS) {
            aError() << "failed to allocate command buffers!";
        }
    }

    // Sync objects
    m_imageAvailableSemaphores.resize(swapChainImageCount);
    m_renderFinishedSemaphores.resize(swapChainImageCount);
    m_inFlightFences.resize(swapChainImageCount);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < swapChainImageCount; i++) {
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
           vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
           vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {

            aError() << "failed to create synchronization objects for a frame!";
        }
    }
}

void SurfaceVk::releaseSwapChain() {
    if(m_swapChain) {
        VkDevice device = WrapperVk::device();

        vkDeviceWaitIdle(device);

        vkDestroySwapchainKHR(device, m_swapChain, nullptr);

        for(size_t i = 0; i < swapChainImageCount; i++) {
            vkDestroySemaphore(device, m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, m_inFlightFences[i], nullptr);
        }
    }
}

bool SurfaceVk::beginFrame(uint32_t width, uint32_t height) {
    VkDevice device = WrapperVk::device();

    if(m_depth->width() != width || m_depth->height() != height) {
        recreateSwapChain();
        if(!m_swapChain) {
            return false;
        }
    }

    vkWaitForFences(device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &m_inFlightFences[m_currentFrame]);

    VkResult result = vkAcquireNextImageKHR(device, m_swapChain, UINT64_MAX,
                                            m_imageAvailableSemaphores[m_currentFrame],
                                            VK_NULL_HANDLE, &m_imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();

        return false;
    } else if(result == VK_ERROR_DEVICE_LOST) {
        releaseSwapChain();

        /// \todo ensure started

        return false;
    }

    return true;
}

void SurfaceVk::endFrame() {
    VkQueue queue = WrapperVk::submit(m_commandBuffers[m_currentFrame], m_imageAvailableSemaphores[m_currentFrame],
                                      m_renderFinishedSemaphores[m_currentFrame], m_inFlightFences[m_currentFrame]);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

    VkSwapchainKHR swapChains[] = { m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_imageIndex;

    VkResult result = vkQueuePresentKHR(queue, &presentInfo);
    if(result != VK_SUCCESS) {
        aError() << "failed to present swap chain image!";
    }

    m_currentFrame = (m_currentFrame + 1) % swapChainImageCount;
}

VkCommandBuffer SurfaceVk::currentCmdBuffer() const {
    return m_commandBuffers[m_currentFrame];
}

void SurfaceVk::setupCurrentTarget(RenderTarget *target) const {
    static_cast<RenderTargetVk *>(target)->setNativeHandle(m_renderPass,
                                                           m_frameBuffers[m_currentFrame],
                                                           m_depth->width(),
                                                           m_depth->height());
}
