#include "resources/rendertargetvk.h"

#include "resources/texturevk.h"

#include <cstring>

#include "wrappervk.h"
#include "commandbuffervk.h"

RenderTargetVk::RenderTargetVk() :
        m_renderPass(VK_NULL_HANDLE),
        m_frameBuffer(VK_NULL_HANDLE),
        m_descriptorPool(VK_NULL_HANDLE),
        m_width(1),
        m_height(1),
        m_native(false),
        m_binded(false) {

}

RenderTargetVk::~RenderTargetVk() {
    VkDevice device = WrapperVk::device();
    for(auto it : m_global) {
        vkDestroyBuffer(device, it.buffer, nullptr);
        vkFreeMemory(device, it.memory, nullptr);
        vkFreeDescriptorSets(device, m_descriptorPool, 1, &it.descriptorSet);
    }
    m_global.clear();

    if(m_descriptorPool) {
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
}

void RenderTargetVk::bind(VkCommandBuffer &buffer, uint32_t level) {
    PROFILE_FUNCTION();

    switch(state()) {
        case Suspend: {
            destroyBuffer();

            setState(ToBeDeleted);
            return;
        }
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
    if(m_binded) {
        vkCmdEndRenderPass(buffer);
        m_binded = false;
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

    m_native = true;
    setState(Ready);
}

void RenderTargetVk::bindBuffer(VkCommandBuffer &buffer) {
    if(m_binded) {
        return;
    }
    uint32_t count = colorAttachmentCount();

    std::vector<VkClearValue> clearValues;
    clearValues.reserve(count + 1);

    int32_t flags = RenderTargetVk::flags();
    if(m_native || flags != 0) {
        for(uint32_t i = 0; i < count; i++) {
            VkClearValue value;
            value.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

            clearValues.push_back(value);
        }

        VkClearValue value;
        value.depthStencil = { 1.0f, 0 };
        clearValues.push_back(value);
    }

    int32_t x, y, w, h;
    renderArea(x, y, w, h);

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = m_renderPass;
    renderPassBeginInfo.framebuffer = m_frameBuffer;
    renderPassBeginInfo.renderArea.offset.x = x;
    renderPassBeginInfo.renderArea.offset.y = y;
    renderPassBeginInfo.renderArea.extent.width = (w == 0) ? m_width : w;
    renderPassBeginInfo.renderArea.extent.height = (h == 0) ? m_height : h;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    if(!clearValues.empty()) {
        renderPassBeginInfo.pClearValues = clearValues.data();
    }

    // move layouts
    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_binded = true;
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
    } else {
        TextureVk *t = static_cast<TextureVk *>(depthAttachment());
        if(t) {
            m_width = t->width();
            m_height = t->height();
        }
    }

    if(m_frameBuffer == nullptr) {
        bool clearColor = (RenderTargetVk::flags() & ClearColor);

        VkDevice device = WrapperVk::device();

        std::vector<VkAttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.reserve(count);

        std::vector<VkImageView> attachments;
        attachments.reserve(count);

        std::vector<VkAttachmentReference> references;

        VkAttachmentDescription description;
        description.samples = VK_SAMPLE_COUNT_1_BIT;
        description.loadOp = clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
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
                description.initialLayout = clearColor ? VK_IMAGE_LAYOUT_UNDEFINED : t->initialLayout();
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

            bool clearDepth = (RenderTargetVk::flags() & ClearDepth);
            description.format = d->vkFormat();
            description.loadOp = clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            description.stencilLoadOp = clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            description.initialLayout = clearDepth ? VK_IMAGE_LAYOUT_UNDEFINED : d->initialLayout();
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

    if(!m_native) {
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

void RenderTargetVk::updateGlobalMemory(size_t currentFrame, const Global &global) {
    VkDevice device = WrapperVk::device();

    size_t swapChainCount = WrapperVk::framesInFlight();
    size_t index = currentFrame;
    if(flags() & Atlas) {
        index += tileIndex() * swapChainCount;
        swapChainCount *= 32;
    }

    if(m_global.empty()) {
        if(m_descriptorPool == VK_NULL_HANDLE) {
            std::vector<VkDescriptorPoolSize> poolSize;
            for(auto &binding : CommandBufferVk::globalLayoutBindings()) {
                poolSize.push_back({ binding.descriptorType, (uint32_t)swapChainCount });
            }
            m_descriptorPool = WrapperVk::createDescriptorPool(poolSize, swapChainCount);
        }

        m_global.resize(swapChainCount);

        uint32_t flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for(uint32_t i = 0; i < swapChainCount; i++) {
            m_global[i].buffer = WrapperVk::createBuffer(sizeof(Global), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            m_global[i].memory = WrapperVk::allocateMemory(flags, m_global[i].buffer);
            m_global[i].descriptorSet = WrapperVk::createDescriptorSet(CommandBufferVk::globalDescriptorSetLayout(), m_descriptorPool);

            for(auto &binding : CommandBufferVk::globalLayoutBindings()) {
                VkWriteDescriptorSet descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_global[i].descriptorSet;
                descriptorWrite.dstBinding = binding.binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = binding.descriptorType;
                descriptorWrite.descriptorCount = 1;

                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = m_global[i].buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = VK_WHOLE_SIZE;

                descriptorWrite.pBufferInfo = &bufferInfo;

                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }
    }

    VkDeviceSize globalSize = sizeof(Global);
    void *dst = nullptr;
    vkMapMemory(device, m_global[index].memory, 0, globalSize, 0, &dst);
        memcpy(dst, &global, globalSize);
    vkUnmapMemory(device, m_global[index].memory);
}

VkDescriptorSet RenderTargetVk::globalDescriptorSet(size_t currentFrame) {
    size_t index = currentFrame;
    if(flags() & Atlas) {
        index += tileIndex() * WrapperVk::framesInFlight();
    }
    return m_global[index].descriptorSet;
}
