#ifndef TEXTUREVK_H
#define TEXTUREVK_H

#include <resources/texture.h>

#include <vulkan/vulkan.h>

class MaterialInstanceVk;
class RenderTargetVk;

class TextureVk : public Texture {
    A_OVERRIDE(TextureVk, Texture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    TextureVk();

    void attributes(VkDescriptorImageInfo &imageinfo);

    VkFormat vkFormat() const;
    VkImageView vkView() const;

    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

    VkImageLayout initialLayout() const;
    VkImageLayout finalLayout() const;

    VkImage createImage(VkDevice device, VkFormat format);

    void setImage(VkDevice device, VkFormat format, VkImage image);

    void destroyImage(VkDevice device);

    void updateTexture();

private:
    uint32_t mipLevel(uint32_t value, uint32_t mip) const;

    void readPixels(CommandBuffer &buffer, int x, int y, int width, int height) override;

    void switchState(State state) override;

    void convertFormatFromNative(VkFormat format);

    void createImageView(VkDevice device, VkFormat format);

    void createSampler(VkDevice device);

    VkSampler m_sampler;

    VkImage m_image;
    VkImageView m_view;

    VkDeviceMemory m_deviceMemory;

    VkImageLayout m_initialLayout;
    VkImageLayout m_finalLayout;

    uint32_t m_deviceWidth;
    uint32_t m_deviceHeight;

};

#endif // TEXTUREVK_H
