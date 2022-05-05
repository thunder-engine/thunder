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

    void attributes(VkImageView &imageView, VkSampler &sampler);

    VkFormat vkFormat() const;

    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    VkImageLayout currentLayout() const;
    void setCurrentLayout(VkImageLayout layout);

private:
    void readPixels(int x, int y, int width, int height) override;

    void switchState(ResourceState state) override;

    void updateTexture();
    void destroyTexture();

    void createImage(VkDevice device, VkFormat format);

    void createImageView(VkDevice device, VkFormat format);

    void createSampler(VkDevice device);

    void copyBufferToImage(VkBuffer buffer);

    VkSampler m_sampler;

    VkImage m_image;
    VkImageView m_view;

    VkDeviceMemory m_deviceMemory;

    VkImageLayout m_currentLayout;

};

#endif // TEXTUREVK_H
