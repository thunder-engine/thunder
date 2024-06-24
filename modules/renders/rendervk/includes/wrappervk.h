#ifndef WRAPPERVK_H
#define WRAPPERVK_H

#include <vulkan/vulkan.h>

#include <vector>

class WrapperVk {
public:
    static VkInstance instance();
    static VkDevice device(VkSurfaceKHR surface = VK_NULL_HANDLE);
    static VkPhysicalDevice physicalDevice();

    static VkCommandPool commandPool();

    static VkQueue submit(VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence);

    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer &buffer);
    static void destroyBuffer(VkBuffer buffer);

    static void allocateMemory(VkMemoryPropertyFlags properties, const VkBuffer &buffer, VkDeviceMemory &memory);
    static void freeMemory(VkDeviceMemory memory);

    static void getStagingBuffer(VkDeviceSize size, VkBuffer &buffer, VkDeviceMemory &memory);

    static VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &layoutBinding);
    static VkDescriptorPool createDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSize, uint32_t count);
    static VkDescriptorSet createDescriptorSet(const VkDescriptorSetLayout &layout, VkDescriptorPool pool);

    static VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout> &layouts);

    static void destroyContext();

    static void createInstance();
    static void selectPhysicalDevice();

    static void recreateDevice(VkSurfaceKHR surface);

    static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

#ifdef SHARED_DEFINE
    static void setupDebugOutput();

#endif

};

#endif // WRAPPERVK_H
