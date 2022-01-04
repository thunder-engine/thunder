#ifndef RENDERVKSYSTEM_H
#define RENDERVKSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

#include <vulkan/vulkan.h>

class Engine;

class RenderVkSystem : public RenderSystem {
public:
    RenderVkSystem(Engine *engine);
    ~RenderVkSystem();

    bool init() override;

    const char *name() const override;

    void update(Scene *scene) override;

    static void setCurrentDevice(VkDevice device);
    static VkDevice currentDevice();

    static void setCurrentPhysicalDevice(VkPhysicalDevice device);
    static VkPhysicalDevice currentPhysicalDevice();

    static void setCurrentCommandPool(VkCommandPool pool);
    static VkCommandPool currentCommandPool();

    static void setCurrentQueue(VkQueue queue);
    static VkQueue currentQueue();

    static void setCurrentRenderPass(VkRenderPass pass);
    static VkRenderPass currentRenderPass();

    static void setCurrentFrameBuffer(VkFramebuffer buffer);

    static void setCommandBuffer(VkCommandBuffer buffer);

    static void setSwapChainImageCount(int32_t count);
    static int32_t swapChainImageCount();

    static void setCurrentSwapChainImageIndex(int32_t index);

    static void setWindowSize(uint32_t width, uint32_t height);

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow() const override;

    vector<uint8_t> renderOffscreen(Scene *scene, int width, int height) override;
#endif

private:
    Engine *m_pEngine;

    bool m_registered;

};

#endif // RENDERVKSYSTEM_H
