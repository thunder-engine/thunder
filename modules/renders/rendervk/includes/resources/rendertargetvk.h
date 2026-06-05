#ifndef RENDERTARGETVK_H
#define RENDERTARGETVK_H

#include <resources/rendertarget.h>

#include <vulkan/vulkan.h>

struct Global;
class CommandBufferVk;

class RenderTargetVk : public RenderTarget {
    A_OBJECT_OVERRIDE(RenderTargetVk, RenderTarget, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderTargetVk();
    ~RenderTargetVk();

    void bind(VkCommandBuffer &buffer, uint32_t level);
    void unbind(VkCommandBuffer &buffer);

    VkRenderPass renderPass() const;

    uint32_t colorAttachmentCount() const override { return m_native ? 1 : RenderTarget::colorAttachmentCount(); }

    void setNativeHandle(VkRenderPass pass, VkFramebuffer buffer, uint32_t width, uint32_t height);

    VkDescriptorSet globalDescriptorSet(size_t currentFrame);
    void updateGlobalMemory(size_t currentFrame, const Global &global);

private:
    void bindBuffer(VkCommandBuffer &buffer);
    bool updateBuffer(uint32_t level);
    void destroyBuffer();

    static void textureUpdated(int state, void *object);

    void switchState(State state) override;

    uint32_t setColorAttachment(uint32_t index, Texture *texture) override;
    void setDepthAttachment(Texture *texture) override;

private:
    struct GlobalResources {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    };

    std::vector<GlobalResources> m_global;

    VkRenderPass m_renderPass;
    VkFramebuffer m_frameBuffer;

    VkDescriptorPool m_descriptorPool;

    uint32_t m_width;
    uint32_t m_height;

    bool m_native;
    bool m_binded;

};

#endif // RENDERTARGETVK_H
