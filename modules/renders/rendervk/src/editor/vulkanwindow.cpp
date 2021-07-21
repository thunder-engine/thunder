#include "editor/vulkanwindow.h"

#include <QVulkanFunctions>

#include "rendervksystem.h"

TriangleRenderer::TriangleRenderer(QVulkanWindow *w)
    : m_window(w) {

}

void TriangleRenderer::initSwapChainResources() {

}

void TriangleRenderer::initResources() {
    m_devFuncs = m_window->vulkanInstance()->deviceFunctions(m_window->device());
}

void TriangleRenderer::releaseSwapChainResources() {
    // Can be called during window minimization
}

void TriangleRenderer::releaseResources() {
    // Can be called during window minimization
}

void TriangleRenderer::startNextFrame() {
    VkClearColorValue clearColor = {{ 0, 0, 0, 1 }};
    VkClearDepthStencilValue clearDS = { 1, 0 };

    VkClearValue clearValues[2];
    memset(clearValues, 0, sizeof(clearValues));
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDS;

    VkRenderPassBeginInfo rpBeginInfo = {};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = m_window->defaultRenderPass();
    rpBeginInfo.framebuffer = m_window->currentFramebuffer();
    const QSize sz = m_window->swapChainImageSize();
    rpBeginInfo.renderArea.extent.width = sz.width();
    rpBeginInfo.renderArea.extent.height = sz.height();
    rpBeginInfo.clearValueCount = 2;
    rpBeginInfo.pClearValues = clearValues;

    VkCommandBuffer cb = m_window->currentCommandBuffer();
    RenderVkSystem::setCommandBuffer(cb);
    RenderVkSystem::setCurrentDevice(m_window->device());
    RenderVkSystem::setCurrentPhysicalDevice(m_window->physicalDevice());
    RenderVkSystem::setCurrentRenderPass(m_window->defaultRenderPass());

    RenderVkSystem::setCurrentCommandPool(m_window->graphicsCommandPool());
    RenderVkSystem::setCurrentQueue(m_window->graphicsQueue());

    int index = m_window->currentSwapChainImageIndex();
    RenderVkSystem::setCurrentColorImage(m_window->swapChainImage(index));
    RenderVkSystem::setCurrentDepthImage(m_window->depthStencilImage());
    RenderVkSystem::setSwapChainImageCount(m_window->swapChainImageCount());
    RenderVkSystem::setCurrentSwapChainImageIndex(index);

    m_devFuncs->vkCmdBeginRenderPass(cb, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    emit static_cast<VulkanWindow *>(m_window)->draw();

    m_devFuncs->vkCmdEndRenderPass(cb);

    m_window->frameReady();
    m_window->requestUpdate();
}

QVulkanWindowRenderer *VulkanWindow::createRenderer() {
    return new TriangleRenderer(this);
}
