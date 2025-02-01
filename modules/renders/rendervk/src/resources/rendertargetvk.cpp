#include "resources/rendertargetvk.h"

#include "resources/texturevk.h"

#include "wrappervk.h"

RenderTargetVk::RenderTargetVk() :
        m_renderPass(VK_NULL_HANDLE),
        m_frameBuffer(VK_NULL_HANDLE),
        m_width(1),
        m_height(1) {

}

void RenderTargetVk::bind(VkCommandBuffer &buffer, uint32_t level) {
    PROFILE_FUNCTION();

    switch(state()) {
        case Suspend: {
            destroyBuffer();

            setState(ToBeDeleted);
            return;
        } break;
        case ToBeUpdated: {
            destroyBuffer();

            if(updateBuffer(level)) {
                setState(Ready);
            }
        } break;
        default: break;
    }

    bindBuffer(buffer);
}

void RenderTargetVk::unbind(VkCommandBuffer &buffer) {
    vkCmdEndRenderPass(buffer);

    // move layouts
}

void RenderTargetVk::clear(VkCommandBuffer &buffer, bool clearColor, const Vector4 &color, bool clearDepth, float depth) {
    PROFILE_FUNCTION();

    uint32_t count = clearColor ? colorAttachmentCount() : 0;

    std::vector<VkClearAttachment> attachments;
    attachments.reserve(count + ((clearDepth) ? 1 : 0));

    std::vector<VkClearRect> rects;
    rects.reserve(count + ((clearDepth) ? 1 : 0));

    VkClearRect rect = {};
    rect.rect.extent = { m_width, m_height};
    rect.layerCount = 1;

    for(uint32_t i = 0; i < count; ++i) {
        VkClearAttachment clearAttachment;
        clearAttachment.clearValue.color = { { color.x, color.y, color.z, color.w } };
        clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachment.colorAttachment = i;

        attachments.push_back(clearAttachment);

        rects.push_back(rect);
    }
    if(clearDepth) {
        VkClearAttachment clearDepth;
        clearDepth.clearValue.depthStencil = { depth, 0 };
        clearDepth.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        attachments.push_back(clearDepth);

        rects.push_back(rect);
    }

    if(!attachments.empty()) {
        vkCmdClearAttachments(buffer, attachments.size(), attachments.data(), rects.size(), rects.data());
    }
}

VkRenderPass RenderTargetVk::renderPass() const {
    return m_renderPass;
}

void RenderTargetVk::setNativeHandle(VkRenderPass pass, VkFramebuffer buffer, uint32_t width, uint32_t height) {
    m_renderPass = pass;
    m_frameBuffer = buffer;
    m_width = width;
    m_height = height;

    makeNative();
    setState(Ready);
}

void RenderTargetVk::bindBuffer(VkCommandBuffer &buffer) {
    uint32_t count = colorAttachmentCount();

    std::vector<VkClearValue> clearValues;
    clearValues.reserve(count + 1);

    for(uint32_t i = 0; i < count; i++) {
        VkClearValue value;
        value.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

        clearValues.push_back(value);
    }
    VkClearValue value;
    value.depthStencil = { 1.0f, 0 };
    clearValues.push_back(value);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass;
    renderPassBeginInfo.framebuffer = m_frameBuffer;
    renderPassBeginInfo.renderArea.extent.width = m_width;
    renderPassBeginInfo.renderArea.extent.height = m_height;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    // move layouts
    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

bool RenderTargetVk::updateBuffer(uint32_t level) {
    PROFILE_FUNCTION();

    uint32_t count = colorAttachmentCount();

    if(count > 0) {
        TextureVk *t = static_cast<TextureVk *>(colorAttachment(0));
        if(t) {
            m_width = t->width();
            m_height = t->height();
        }
    }

    if(m_frameBuffer == nullptr) {
        //bool clearOnBind = false;

        VkDevice device = WrapperVk::device();

        std::vector<VkAttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.reserve(count);

        std::vector<VkImageView> attachments;
        attachments.reserve(count);

        std::vector<VkAttachmentReference> references;

        VkAttachmentDescription description;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = /*clearOnBind ? VK_ATTACHMENT_LOAD_OP_CLEAR :*/ VK_ATTACHMENT_LOAD_OP_LOAD;
        description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        description.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;

        uint32_t i;
        for(i = 0; i < count; i++) {
            description.format = VK_FORMAT_R8G8B8A8_UNORM;
            TextureVk *t = static_cast<TextureVk *>(colorAttachment(i));
            if(t) {
                VkDescriptorImageInfo imageInfo = {};
                t->attributes(imageInfo);
                attachments.push_back(imageInfo.imageView);

                description.format = t->vkFormat();
                description.initialLayout = /*clearOnBind ? VK_IMAGE_LAYOUT_UNDEFINED :*/ t->initialLayout();
                description.finalLayout = t->finalLayout();
            }

            attachmentDescriptions.push_back(description);
            references.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        }

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = references.size();
        subpassDescription.pColorAttachments = references.data();

        subpassDescription.pDepthStencilAttachment = nullptr;
        TextureVk *d = static_cast<TextureVk *>(depthAttachment());
        if(d) {
            VkAttachmentReference depthReference = { i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
            subpassDescription.pDepthStencilAttachment = &depthReference;

            VkDescriptorImageInfo imageInfo = {};
            d->attributes(imageInfo);
            attachments.push_back(imageInfo.imageView);

            description.format = d->vkFormat();
            description.loadOp = /*clearOnBind ? VK_ATTACHMENT_LOAD_OP_CLEAR :*/ VK_ATTACHMENT_LOAD_OP_LOAD;
            description.stencilLoadOp = /*clearOnBind ? VK_ATTACHMENT_LOAD_OP_CLEAR :*/ VK_ATTACHMENT_LOAD_OP_LOAD;
            description.initialLayout = /*clearOnBind ? VK_IMAGE_LAYOUT_UNDEFINED :*/ d->initialLayout();
            description.finalLayout = d->finalLayout();

            attachmentDescriptions.push_back(description);
        }

        std::vector<VkSubpassDependency> dependencies;
        dependencies.resize(2);

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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

    return true;
}

void RenderTargetVk::destroyBuffer() {
    PROFILE_FUNCTION();

    if(!isNative()) {
        VkDevice device = WrapperVk::device();

        if(m_renderPass) {
            vkDestroyRenderPass(device, m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }

        if(m_frameBuffer) {
            vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
            m_frameBuffer = VK_NULL_HANDLE;
        }
    }
}

void RenderTargetVk::switchState(State state) {
    switch(state) {
        case Unloading: {
            destroyBuffer();
        } break;
        default: RenderTarget::switchState(state); break;
    }
}

void RenderTargetVk::textureUpdated(int state, void *object) {
    if(state == Resource::ToBeUpdated) {
        RenderTargetVk *target = reinterpret_cast<RenderTargetVk *>(object);
        target->switchState(ToBeUpdated);
    }
}

uint32_t RenderTargetVk::setColorAttachment(uint32_t index, Texture *texture) {
    if(colorAttachment(index) != texture) {
        Texture *old = colorAttachment(index);
        if(old) {
            old->unsubscribe(this);
        }
        texture->subscribe(&RenderTargetVk::textureUpdated, this);

        switchState(ToBeUpdated);
        return RenderTarget::setColorAttachment(index, texture);
    }
    return index;
}

void RenderTargetVk::setDepthAttachment(Texture *texture) {
    if(depthAttachment() != texture) {
        Texture *old = depthAttachment();
        if(old) {
            old->unsubscribe(this);
        }
        texture->subscribe(&RenderTargetVk::textureUpdated, this);

        switchState(ToBeUpdated);
        RenderTarget::setDepthAttachment(texture);
    }
}
