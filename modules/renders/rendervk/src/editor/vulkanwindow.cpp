#include "editor/vulkanwindow.h"

#include "rendervksystem.h"

VulkanWindowRenderer::VulkanWindowRenderer(QVulkanWindow *w)
    : m_window(w) {


}

void VulkanWindowRenderer::initSwapChainResources() {

}

void VulkanWindowRenderer::initResources() {

}

void VulkanWindowRenderer::releaseSwapChainResources() {
    // Can be called during window minimization
}

void VulkanWindowRenderer::releaseResources() {
    // Can be called during window minimization
}

void VulkanWindowRenderer::startNextFrame() {
    static_cast<VulkanWindow *>(m_window)->render();
}

void VulkanWindow::render() {
    RenderVkSystem::setCommandBuffer(currentCommandBuffer());
    RenderVkSystem::setCurrentDevice(device());
    RenderVkSystem::setCurrentPhysicalDevice(physicalDevice());
    RenderVkSystem::setCurrentRenderPass(defaultRenderPass());
    RenderVkSystem::setCurrentFrameBuffer(currentFramebuffer());

    RenderVkSystem::setCurrentCommandPool(graphicsCommandPool());
    RenderVkSystem::setCurrentQueue(graphicsQueue());

    RenderVkSystem::setSwapChainImageCount(swapChainImageCount());
    RenderVkSystem::setCurrentSwapChainImageIndex(currentSwapChainImageIndex());

    RenderVkSystem::setWindowSize(width(), height());

    emit draw();

    frameReady();
    requestUpdate();
}

QVulkanWindowRenderer *VulkanWindow::createRenderer() {
    return new VulkanWindowRenderer(this);
}
