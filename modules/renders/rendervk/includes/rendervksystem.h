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

    void registerClasses() override;
    void unregisterClasses() override;

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

    static void setCommandBuffer(VkCommandBuffer buffer);
    static void setCurrentColorImage(VkImage image);
    static void setCurrentDepthImage(VkImage image);

    static void setSwapChainImageCount(int32_t count);
    static int32_t swapChainImageCount();

    static void setCurrentSwapChainImageIndex(int32_t index);

#ifdef NEXT_SHARED
    QWindow *createRhiWindow() const override;
#endif

private:
    Engine *m_pEngine;

    bool m_registered;

};

#endif // RENDERVKSYSTEM_H
