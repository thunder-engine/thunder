#include "resources/texturevk.h"

#include <cstring>

#include "wrappervk.h"

#include "resources/materialvk.h"
#include "resources/rendertarget.h"

/*
// Depth
const VkFormat depthFormatCandidates[] = {
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D16_UNORM_S8_UINT
};

int dsFormatIdx = 0;
while(dsFormatIdx < sizeof(depthFormatCandidates) / sizeof(VkFormat)) {
    s_depthStencilFormat = depthFormatCandidates[dsFormatIdx];
    VkFormatProperties fmtProp;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, s_depthStencilFormat, &fmtProp);
    if(fmtProp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        break;
    }
    ++dsFormatIdx;
}
*/
TextureVk::TextureVk() :
        m_sampler(VK_NULL_HANDLE),
        m_image(VK_NULL_HANDLE),
        m_view(VK_NULL_HANDLE),
        m_deviceMemory(VK_NULL_HANDLE),
        m_initialLayout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_finalLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        m_deviceWidth(0),
        m_deviceHeight(0) {

}

void TextureVk::attributes(VkDescriptorImageInfo &imageinfo) {
    PROFILE_FUNCTION();

    switch(state()) {
        case ToBeUpdated: {
            updateTexture();
            setState(Ready);
        } break;
        default: break;
    }

    imageinfo.imageView = m_view;
    imageinfo.sampler = m_sampler;
    imageinfo.imageLayout = m_initialLayout;
}

VkFormat TextureVk::vkFormat() const {
    VkFormat result = VK_FORMAT_R8G8B8A8_UNORM;

    if(m_compress == Uncompressed) {
        switch(format()) {
        case R8: result = VK_FORMAT_R8_UNORM; break;
        case RGB10A2: result = VK_FORMAT_A2R10G10B10_UNORM_PACK32; break;
        case RGBA32Float: result = VK_FORMAT_R32G32B32A32_SFLOAT; break;
        case RGBA16Float: result = VK_FORMAT_R16G16B16A16_SFLOAT; break;
        case R11G11B10Float: result = VK_FORMAT_B10G11R11_UFLOAT_PACK32; break;
        case Depth: result = (depthBits() == 16) ? VK_FORMAT_D16_UNORM_S8_UINT : VK_FORMAT_D24_UNORM_S8_UINT; break;
        default: break;
        }
    } else {
        switch(m_compress) {
        case BC1: result = VK_FORMAT_BC1_RGB_UNORM_BLOCK; break;
        case BC3: result = VK_FORMAT_BC3_UNORM_BLOCK; break;
        case BC7: result = VK_FORMAT_BC7_UNORM_BLOCK; break;
        case ASTC: result = VK_FORMAT_ASTC_4x4_UNORM_BLOCK; break;
        case ETC1: result = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK; break;
        case ETC2: result = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK; break;
        case PVRTC: result = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG; break;
        default: break;
        }
    }

    return result;
}

VkImageView TextureVk::vkView() const {
    return m_view;
}

void TextureVk::readPixels(int x, int y, int width, int height) {
    if(sides() != 0) {
        bool depth = (TextureVk::format() == Depth);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {x, y, 0};
        region.imageExtent = {(uint32_t)width, (uint32_t)height, 1};

        VkDeviceSize textureSize = sizeRGB(width, height, 1);

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        WrapperVk::getStagingBuffer(textureSize, stagingBuffer, stagingMemory);

        VkImageLayout original = depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        transitionImageLayout(original, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkCommandBuffer commandBuffer = WrapperVk::beginSingleTimeCommands();

        vkCmdCopyImageToBuffer(commandBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

        WrapperVk::endSingleTimeCommands(commandBuffer);

        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, original);

        VkDevice device = WrapperVk::device();

        Surface &dst = surface(0);

        uint8_t *src = nullptr;
        vkMapMemory(device, stagingMemory, 0, textureSize, 0, reinterpret_cast<void **>(&src));
            memcpy(dst[0].data(), src, textureSize);
        vkUnmapMemory(device, stagingMemory);
    }
}

void TextureVk::switchState(State state) {
    switch(state) {
        case Unloading: {
            VkDevice device = WrapperVk::device();

            destroyImage(device);

            if(m_sampler) {
                vkDestroySampler(device, m_sampler, nullptr);
                m_sampler = nullptr;
            }
        } break;
        default: Texture::switchState(state); break;
    }
}

void TextureVk::updateTexture() {
    VkDevice device = WrapperVk::device();

    VkFormat vkformat = vkFormat();

    if(m_image == nullptr || m_width != m_deviceWidth || m_height != m_deviceHeight) {
        if(m_image) {
            destroyImage(device);
        }
        m_deviceWidth = m_width;
        m_deviceHeight = m_height;

        createImage(device, vkformat);
        m_initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if(!isRender()) {
        transitionImageLayout(m_initialLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        for(int side = 0; side < sides(); side++) {
            const Surface &src = surface(side);

            VkDeviceSize textureSize = 0;
            for(uint32_t mip = 0; mip < src.size(); mip++) {
                textureSize += src[mip].size();
            }

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;
            WrapperVk::getStagingBuffer(textureSize, stagingBuffer, stagingMemory);

            uint8_t *dst = nullptr;
            vkMapMemory(device, stagingMemory, 0, textureSize, 0, reinterpret_cast<void **>(&dst));
            for(uint32_t mip = 0; mip < src.size(); mip++) {
                uint32_t mipSize = src[mip].size();
                memcpy(dst, src[mip].data(), mipSize);
                dst += mipSize;
            }
            vkUnmapMemory(device, stagingMemory);

            VkCommandBuffer commandBuffer = WrapperVk::beginSingleTimeCommands();

            VkDeviceSize offset = 0;
            for(uint32_t mip = 0; mip < src.size(); mip++) {
                uint32_t w = mipLevel(m_width, mip);
                uint32_t h = mipLevel(m_height, mip);
                uint32_t d = mipLevel(m_depth, mip);

                VkDeviceSize mipSize = src[mip].size();

                VkBufferImageCopy region = {};
                region.bufferOffset = offset;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = mip;
                region.imageSubresource.baseArrayLayer = side;
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { w, h, d };

                vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                offset += mipSize;
            }

            WrapperVk::endSingleTimeCommands(commandBuffer);
        }

        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_finalLayout);

    } else {
        if(TextureVk::format() == Depth) {
            m_initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            m_finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else {
            m_initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            m_finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, m_initialLayout);
    }

    createSampler(device);

    createImageView(device, vkformat);
}

void TextureVk::destroyImage(VkDevice device) {
    if(m_view) {
        vkDestroyImageView(device, m_view, nullptr);
        m_view = VK_NULL_HANDLE;
    }

    if(m_image) {
        vkDestroyImage(device, m_image, nullptr);
        m_image = VK_NULL_HANDLE;
    }

    if(m_deviceMemory) {
        vkFreeMemory(device, m_deviceMemory, nullptr);
        m_deviceMemory = VK_NULL_HANDLE;
    }
}

VkImage TextureVk::createImage(VkDevice device, VkFormat format) {
    convertFormatFromNative(format);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = (m_depth == 1) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = m_depth;
    imageInfo.mipLevels = mipCount();
    imageInfo.arrayLayers = MAX(m_sides.size(), 1);
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.format = format;

    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if(!isRender()) {
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if(isCubemap()) {
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }
    } else {
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

        if(isFeedback()) {
            imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if(TextureVk::format() == Depth) {
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }

    if(vkCreateImage(device, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = WrapperVk::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if(vkAllocateMemory(device, &allocInfo, nullptr, &m_deviceMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, m_image, m_deviceMemory, 0);

    return m_image;
}

void TextureVk::createImageView(VkDevice device, VkFormat format) {
    if(m_view) {
        vkDestroyImageView(device, m_view, nullptr);
        m_view = VK_NULL_HANDLE;
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = (m_depth == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
    if(isCubemap()) {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = (TextureVk::format() == Depth) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipCount();
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = MAX(m_sides.size(), 1);

    if(vkCreateImageView(device, &viewInfo, nullptr, &m_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
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
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void TextureVk::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    if(oldLayout == newLayout) {
        return;
    }
    VkCommandBuffer commandBuffer = WrapperVk::beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = (format() == Depth) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipCount();
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = MAX(sides(), 1);

    VkPipelineStageFlags sourceStage = 0;
    VkPipelineStageFlags destinationStage = 0;

    switch(oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED: {
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        } break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        } break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } break;
        default: break;
    }

    switch(newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } break;
        default: break;
    }

    if(sourceStage == 0 || destinationStage == 0) {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    WrapperVk::endSingleTimeCommands(commandBuffer);

    m_initialLayout = newLayout;
}

VkImageLayout TextureVk::initialLayout() const {
    return m_initialLayout;
}

VkImageLayout TextureVk::finalLayout() const {
    return m_finalLayout;
}

void TextureVk::setImage(VkDevice device, VkFormat format, VkImage image) {
    m_image = image;

    convertFormatFromNative(format);

    createImageView(device, format);

    setState(Resource::ToBeUpdated);
}

void TextureVk::convertFormatFromNative(VkFormat format) {
    switch(format) {
    case VK_FORMAT_R8G8B8A8_UNORM: setFormat(RGBA8); break;
    case VK_FORMAT_R8_UNORM: setFormat(R8); break;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32: setFormat(RGB10A2); break;
    case VK_FORMAT_R32G32B32A32_SFLOAT: setFormat(RGBA32Float); break;
    case VK_FORMAT_R16G16B16A16_SFLOAT: setFormat(RGBA16Float); break;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32: setFormat(R11G11B10Float); break;
    case VK_FORMAT_D16_UNORM_S8_UINT: {
        setFormat(Depth);
        setDepthBits(16);
        break;
    }
    case VK_FORMAT_D24_UNORM_S8_UINT: {
        setFormat(Depth);
        setDepthBits(24);
        break;
    }
    default: break;
    }
}

uint32_t TextureVk::mipLevel(uint32_t value, uint32_t mip) const {
    uint32_t result = value >> mip;
    return result > 0 ? result : 1;
}
