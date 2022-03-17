#include "resources/rendertargetvk.h"

#include "resources/texturevk.h"

#include "rendervksystem.h"

RenderTargetVk::RenderTargetVk() :
        m_renderPass(nullptr),
        m_frameBuffer(nullptr),
        m_width(1),
        m_height(1) {

}

void RenderTargetVk::bind(VkCommandBuffer &buffer, uint32_t level) {
    PROFILE_FUNCTION();

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

void RenderTargetVk::unbind(VkCommandBuffer &buffer) {
    vkCmdEndRenderPass(buffer);
}

void RenderTargetVk::clear(VkCommandBuffer &buffer, bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILE_FUNCTION();

    uint32_t count = colorAttachmentCount();

    vector<VkClearAttachment> attachments;
    attachments.reserve(count);

    vector<VkClearRect> rects;
    attachments.reserve(count);

    for(uint32_t i = 0; i < count; ++i) {
        VkClearAttachment clearAttachment;
        clearAttachment.clearValue.color = { { color.x, color.y, color.z, color.w } };
        clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachment.colorAttachment = 0;

        attachments.push_back(clearAttachment);

        VkClearRect rect = {};
        rect.rect.extent = { m_width, m_height};
        rect.layerCount = 1;
        rects.push_back(rect);
    }

    vkCmdClearAttachments(buffer, attachments.size(), attachments.data(), rects.size(), rects.data());
}

void RenderTargetVk::setNativeHandle(VkRenderPass pass, VkFramebuffer buffer, uint32_t width, uint32_t height) {
    m_renderPass = pass;
    m_frameBuffer = buffer;
    m_width = width;
    m_height = height;

    makeNative();
}

bool RenderTargetVk::updateBuffer(VkCommandBuffer &buffer, uint32_t level) {
    PROFILE_FUNCTION();

    uint32_t count = colorAttachmentCount();

    if(count > 0) {
        TextureVk *t = static_cast<TextureVk *>(colorAttachment(0));
        if(t) {
            m_width = t->width();
            m_height = t->height();
        }
    }

    vector<VkClearValue> clearValues;
    clearValues.reserve(count + 1);

    for(uint32_t i = 0; i < count; i++) {
        VkClearValue value;
        value.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

        clearValues.push_back(value);
    }
    VkClearValue value;
    value.depthStencil = { 1.0f, 0 };
    clearValues.push_back(value);

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

                attachmentDescriptions[i].format = t->vkFormat();
            }

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
        fbufCreateInfo.width = m_width;
        fbufCreateInfo.height = m_height;
        fbufCreateInfo.layers = 1;

        vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &m_frameBuffer);
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass;
    renderPassBeginInfo.framebuffer = m_frameBuffer;
    renderPassBeginInfo.renderArea.extent.width = m_width;
    renderPassBeginInfo.renderArea.extent.height = m_height;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

void RenderTargetVk::destroyBuffer() {
    PROFILE_FUNCTION();

    VkDevice device = RenderVkSystem::currentDevice();

    vkDestroyRenderPass(device, m_renderPass, nullptr);
    vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
}
