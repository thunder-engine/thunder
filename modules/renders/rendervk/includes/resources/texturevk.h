#ifndef TEXTUREVK_H
#define TEXTUREVK_H

#include <resources/texture.h>

#include <vulkan/vulkan.h>

class TextureVk : public Texture {
    A_OVERRIDE(TextureVk, Texture, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    TextureVk();

    void attributes(VkDescriptorImageInfo &imageinfo);

    VkFormat vkFormat() const;
    VkImageView vkView() const;

    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    VkImageLayout initialLayout() const;
    VkImageLayout finalLayout() const;

    VkImage createImage(VkDevice device, VkFormat format);

    void setImage(VkImage image, VkFormat format);

private:
    uint32_t mipWidth(uint32_t mip) const;
    uint32_t mipHeight(uint32_t mip) const;

    void readPixels(int x, int y, int width, int height) override;

    void switchState(State state) override;

    void updateTexture();
    void destroyTexture();

    void convertFormatFromNative(VkFormat format);

    void createImageView(VkDevice device, VkFormat format);

    void createSampler(VkDevice device);

    VkSampler m_sampler;

    VkImage m_image;
    VkImageView m_view;

    VkDeviceMemory m_deviceMemory;

    VkImageLayout m_initialLayout;
    VkImageLayout m_finalLayout;

};

#endif // TEXTUREVK_H
