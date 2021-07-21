#include "resources/rendertargetvk.h"

#include "resources/texturevk.h"

#include "rendervksystem.h"

RenderTargetVk::RenderTargetVk() :
    m_renderPass(nullptr),
    m_frameBuffer(nullptr) {

}

void RenderTargetVk::bindTarget(VkCommandBuffer &buffer, uint32_t level) {
    switch(state()) {
        case Suspend: {
            destroyBuffer();

            setState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            if(updateBuffer(buffer, level)) {
                setState(Ready);
                return;
            }
        } break;
        default: break;
    }

    updateBuffer(buffer, level);
}

bool RenderTargetVk::updateBuffer(VkCommandBuffer &buffer, uint32_t level) {
    uint32_t width = 1;
    uint32_t height = 1;

    uint32_t count = colorAttachmentCount();

    if(count > 0) {
        TextureVk *t = static_cast<TextureVk *>(colorAttachment(0));
        if(t) {
            width = t->width();
            height = t->height();
        }
    }

    if(m_renderPass == nullptr) {
        VkDevice device = RenderVkSystem::currentDevice();

        vector<VkAttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.resize(count);

        vector<VkSubpassDependency> dependencies;
        dependencies.resize(count);

        vector<VkImageView> attachments;
        attachments.resize(count);

        for(uint32_t i = 0; i < count; i++) {
            TextureVk *t = static_cast<TextureVk *>(colorAttachment(i));

            if(t) {
                VkImageView view;
                VkSampler sampler;
                t->attributes(view, sampler);
                attachments[i] = view;
            }

            attachmentDescriptions[i].format = t->vkFormat();
            attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            dependencies[i].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[i].dstSubpass = 0;
            dependencies[i].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[i].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[i].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }
/*
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
*/
        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = nullptr;//&depthReference;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachmentDescriptions.size();
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

        vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass);

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = m_renderPass;
        fbufCreateInfo.attachmentCount = attachments.size();
        fbufCreateInfo.pAttachments = attachments.data();
        fbufCreateInfo.width = width;
        fbufCreateInfo.height = height;
        fbufCreateInfo.layers = 1;

        vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &m_frameBuffer);
    }

    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass;
    renderPassBeginInfo.framebuffer = m_frameBuffer;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

void RenderTargetVk::destroyBuffer() {
    VkDevice device = RenderVkSystem::currentDevice();

    vkDestroyRenderPass(device, m_renderPass, nullptr);
    vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
}
