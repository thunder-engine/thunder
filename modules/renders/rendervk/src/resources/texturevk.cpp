#include "resources/texturevk.h"

#include "rendervksystem.h"

#include "commandbuffervk.h"

TextureVk::TextureVk() :
        m_sampler(nullptr),
        m_image(nullptr),
        m_view(nullptr),
        m_deviceMemory(nullptr),
        m_useStaging(false) {

}

void TextureVk::attributes(VkImageView &imageView, VkSampler &sampler) {
    switch(state()) {
        case Suspend: {
            destroyTexture();
            setState(ToBeDeleted);
        } break;
        case ToBeUpdated: {
            updateTexture();
            setState(Ready);
        } break;
        default: break;
    }

    imageView = m_view;
    sampler = m_sampler;
}

void TextureVk::switchState(ResourceState state) {
    setState(state);
}

VkFormat TextureVk::vkFormat() const {
    VkFormat result = VK_FORMAT_R8G8B8A8_UNORM;
    switch(format()) {
        case R8: {
            result = VK_FORMAT_R8_UNORM;
        } break;
        case RGB8: {
            result = VK_FORMAT_R8G8B8_UNORM;
        } break;
        case RGB10A2: {
            result = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        } break;
        case RGB16Float: {
            result = VK_FORMAT_R16G16B16_SFLOAT;
        } break;
        case RGBA32Float: {
            result = VK_FORMAT_R32G32B32A32_SFLOAT;
        } break;
        case R11G11B10Float: {
            result = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        } break;
        case Depth: {
            result = (depthBits() == 16) ? VK_FORMAT_D16_UNORM : VK_FORMAT_D24_UNORM_S8_UINT;
        } break;
        default: break;
    }

    return result;
}

void TextureVk::readPixels(int x, int y, int width, int height) {
    /// \todo Unfinished part
}

void TextureVk::updateTexture() {
    VkDevice device = RenderVkSystem::currentDevice();

    VkFormat vkformat = vkFormat();

    createImage(device, vkformat);

    Texture::Sides *sides = getSides();

    if(!sides->empty()) {
        const Surface &image = sides->at(0);
        int mip = 0; // current mip level

        VkDeviceSize textureSize = size(width(), height());
        if(m_useStaging) {
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            CommandBufferVk::createBuffer(textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

            void *dst = nullptr;
            vkMapMemory(device, stagingMemory, 0, textureSize, 0, &dst);
                memcpy(dst, image[mip].data(), textureSize);
            vkUnmapMemory(device, stagingMemory);

            transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
                copyBufferToImage(stagingBuffer);
            transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

            vkFreeMemory(device, stagingMemory, nullptr);
            vkDestroyBuffer(device, stagingBuffer, nullptr);
        } else {
            void *dst = nullptr;
            vkMapMemory(device, m_deviceMemory, 0, textureSize, 0, &dst);
                memcpy(dst, image[mip].data(), textureSize);
            vkUnmapMemory(device, m_deviceMemory);
        }
    }

    createSampler(device);

    createImageView(device, vkformat);
}

void TextureVk::destroyTexture() {
    VkDevice device = RenderVkSystem::currentDevice();

    vkDestroyImageView(device, m_view, nullptr);
    vkDestroyImage(device, m_image, nullptr);
    vkFreeMemory(device, m_deviceMemory, nullptr);
}

void TextureVk::createImage(VkDevice device, VkFormat format) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width();
    imageInfo.extent.height = height();
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.format = format;

    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    bool isFrameBuffer = getSides()->empty();

    if(!isFrameBuffer) {
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        imageInfo.tiling = (m_useStaging) ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;

        if(m_useStaging) {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
    } else {
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if(vkCreateImage(device, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
        throw runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    VkMemoryPropertyFlags memFlags = (isFrameBuffer || m_useStaging) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT :
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocInfo.memoryTypeIndex = CommandBufferVk::findMemoryType(memRequirements.memoryTypeBits, memFlags);

    if(vkAllocateMemory(device, &allocInfo, nullptr, &m_deviceMemory) != VK_SUCCESS) {
        throw runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, m_image, m_deviceMemory, 0);
}

void TextureVk::createImageView(VkDevice device, VkFormat format) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if(vkCreateImageView(device, &viewInfo, nullptr, &m_view) != VK_SUCCESS) {
        throw runtime_error("failed to create texture image view!");
    }
}

void TextureVk::createSampler(VkDevice device) {
    VkFilter min = VK_FILTER_NEAREST;
    VkFilter mag = VK_FILTER_NEAREST;
    switch(filtering()) {
        case Bilinear:
        case Trilinear: mag = VK_FILTER_LINEAR; min = VK_FILTER_LINEAR; break;
        default: break;
    }

    VkSamplerAddressMode mode;
    switch(wrap()) {
        case Repeat:   mode = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
        case Mirrored: mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
        default:       mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = mag;
    samplerInfo.minFilter = min;
    samplerInfo.addressModeU = mode;
    samplerInfo.addressModeV = mode;
    samplerInfo.addressModeW = mode;
    samplerInfo.anisotropyEnable = VK_FALSE;//VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;//properties.limits.maxSamplerAnisotropy;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if(vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw runtime_error("failed to create texture sampler!");
    }
}

void TextureVk::copyBufferToImage(VkBuffer buffer) {
    VkCommandBuffer commandBuffer = CommandBufferVk::beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        (uint32_t)width(),
        (uint32_t)height(),
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    CommandBufferVk::endSingleTimeCommands(commandBuffer);
}

void TextureVk::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = CommandBufferVk::beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    CommandBufferVk::endSingleTimeCommands(commandBuffer);
}
