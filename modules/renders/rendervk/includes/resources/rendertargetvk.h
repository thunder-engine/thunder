#ifndef RENDERTARGETVK_H
#define RENDERTARGETVK_H

#include <resources/rendertarget.h>

#include <vulkan/vulkan.h>

class RenderTargetVk : public RenderTarget {
    A_OVERRIDE(RenderTargetVk, RenderTarget, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetVk();

    void bind(VkCommandBuffer &buffer, uint32_t level);
    void unbind(VkCommandBuffer &buffer);

    VkRenderPass renderPass() const;

    void setNativeHandle(VkRenderPass pass, VkFramebuffer buffer, uint32_t width, uint32_t height);

private:
    void bindBuffer(VkCommandBuffer &buffer);
    bool updateBuffer(uint32_t level);
    void destroyBuffer();

    static void textureUpdated(int state, void *object);

    void switchState(State state) override;

    uint32_t setColorAttachment(uint32_t index, Texture *texture) override;
    void setDepthAttachment(Texture *texture) override;

    VkRenderPass m_renderPass;
    VkFramebuffer m_frameBuffer;

    uint32_t m_width;
    uint32_t m_height;

    bool m_native;

};

#endif // RENDERTARGETVK_H
