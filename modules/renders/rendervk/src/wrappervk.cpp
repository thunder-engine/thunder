#include "wrappervk.h"

#include <log.h>
#include <engine.h>

#include <set>

#if defined(SHARED_DEFINE)
#include <QDebug>

VkDebugUtilsMessengerEXT s_debugMessenger;
#endif

static VkInstance s_instance;
static VkPhysicalDevice s_physicalDevice = VK_NULL_HANDLE;
static VkDevice s_device = VK_NULL_HANDLE;

static VkCommandPool s_commandPool = VK_NULL_HANDLE;

static uint32_t s_queueFamilyIndex = uint32_t(-1);
static uint32_t s_queueIndex = 0;

namespace {
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

VkInstance WrapperVk::instance() {
    return s_instance;
}

VkDevice WrapperVk::device(VkSurfaceKHR surface) {
    if(s_device == VK_NULL_HANDLE) {
        recreateDevice(surface);
    }

    return s_device;
}

VkPhysicalDevice WrapperVk::physicalDevice() {
    return s_physicalDevice;
}

VkCommandPool WrapperVk::commandPool() {
    if(s_commandPool == VK_NULL_HANDLE) {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = s_queueFamilyIndex;

        VkResult result = vkCreateCommandPool(s_device, &poolInfo, nullptr, &s_commandPool);
        if(result != VK_SUCCESS) {
            aError() << "failed to create command pool!";
        }
    }

    return s_commandPool;
}

VkQueue WrapperVk::submit(VkCommandBuffer commandBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence) {
    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(s_device, s_queueFamilyIndex, s_queueIndex, &queue);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
    if(result != VK_SUCCESS) {
        aError() << "failed to submit draw command buffer!";
    }

    return queue;
}

VkCommandBuffer WrapperVk::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = s_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(s_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void WrapperVk::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(s_device, s_queueFamilyIndex, s_queueIndex, &queue);
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(s_device, s_commandPool, 1, &commandBuffer);
}

uint32_t WrapperVk::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(s_physicalDevice, &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return 0;
}

void WrapperVk::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer &buffer) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(s_device, &bufferInfo, nullptr, &buffer);
    if(result != VK_SUCCESS) {
        aError() << "failed to crate buffer!";
    }
}

void WrapperVk::destroyBuffer(VkBuffer buffer) {
    if(buffer) {
        vkDestroyBuffer(s_device, buffer, nullptr);
    }
}

void WrapperVk::allocateMemory(VkMemoryPropertyFlags properties, const VkBuffer &buffer, VkDeviceMemory &memory) {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(s_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    VkResult result = vkAllocateMemory(s_device, &allocInfo, nullptr, &memory);

    if(result != VK_SUCCESS) {
        aError() << "failed to allocate memory!";
    }

    vkBindBufferMemory(s_device, buffer, memory, 0);
}

void WrapperVk::freeMemory(VkDeviceMemory memory) {
    if(memory) {
        vkFreeMemory(s_device, memory, nullptr);
    }
}

void WrapperVk::getStagingBuffer(VkDeviceSize size, VkBuffer &buffer, VkDeviceMemory &memory) {
    static VkBuffer stagingBuffer = nullptr;
    static VkDeviceMemory stagingMemory = nullptr;
    static VkDeviceSize bufferSize = 0;

    if(bufferSize < size) {
        destroyBuffer(stagingBuffer);
        freeMemory(stagingMemory);

        createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, stagingBuffer);
        allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

        bufferSize = size;
    }

    buffer = stagingBuffer;
    memory = stagingMemory;
}

VkDescriptorSetLayout WrapperVk::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding> &layoutBinding) {
    VkDescriptorSetLayoutCreateInfo descLayoutInfo = {};
    descLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descLayoutInfo.bindingCount = layoutBinding.size();
    descLayoutInfo.pBindings = layoutBinding.data();

    VkDescriptorSetLayout descSetLayout;
    VkResult result = vkCreateDescriptorSetLayout(s_device, &descLayoutInfo, nullptr, &descSetLayout);

    if(result != VK_SUCCESS) {
        aError() << "failed to create descriptor set layout!";
    }

    return descSetLayout;
}

VkDescriptorPool WrapperVk::createDescriptorPool(const std::vector<VkDescriptorPoolSize> &poolSize, uint32_t count) {
    VkDescriptorPool result;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSize.size();
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = count;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if(vkCreateDescriptorPool(s_device, &poolInfo, nullptr, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return result;
}

VkDescriptorSet WrapperVk::createDescriptorSet(const VkDescriptorSetLayout &layout, VkDescriptorPool pool) {
    VkDescriptorSet result;

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if(vkAllocateDescriptorSets(s_device, &allocInfo, &result) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return result;
}

VkPipelineLayout WrapperVk::createPipelineLayout(const std::vector<VkDescriptorSetLayout> &layouts) {
    VkPipelineLayout result;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    if(vkCreatePipelineLayout(WrapperVk::device(), &pipelineLayoutInfo, nullptr, &result) != VK_SUCCESS) {
        aWarning() << "Unable to create plipeline layout.";
    }

    return result;
}

void WrapperVk::destroyContext() {
    vkDestroyCommandPool(s_device, s_commandPool, nullptr);
    vkDestroyDevice(s_device, nullptr);
    vkDestroyInstance(s_instance, nullptr);

#if defined(SHARED_DEFINE)
    if(s_debugMessenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(s_instance, "vkDestroyDebugUtilsMessengerEXT"));

        vkDestroyDebugUtilsMessengerEXT(s_instance, s_debugMessenger, nullptr);
    }
#endif
}

void WrapperVk::createInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Engine::value(".project").toString().data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Thunder Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    const std::vector<const char *> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,

    #if defined (_WIN32)
        "VK_KHR_win32_surface",
    #elif defined (__linux__)
        "VK_KHR_xlib_surface",
    #elif defined (__APPLE__)
        "VK_MVK_macos_surface",
    #endif

    #if defined (SHARED_DEFINE)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    #endif
    };

    const std::vector<const char*> layers = {
    #if defined (SHARED_DEFINE)
        "VK_LAYER_KHRONOS_validation"
    #endif
    };

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &s_instance);

    if(result != VK_SUCCESS) {
        aError() << "failed to create instance!";
    }

#if defined (SHARED_DEFINE)
    setupDebugOutput();
#endif
}

void WrapperVk::selectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(s_instance, &deviceCount, nullptr);

    if(deviceCount > 0) {
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(s_instance, &deviceCount, devices.data());

        //VkPhysicalDeviceProperties deviceProperties;
        //VkPhysicalDeviceFeatures deviceFeatures;

        for(auto &device : devices) {
            //vkGetPhysicalDeviceProperties(device, &deviceProperties);
            //vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            if(checkDeviceExtensionSupport(device)) {
                s_physicalDevice = device;

                break;
            }
        }
    }
}

void WrapperVk::recreateDevice(VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(s_physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(s_physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::vector<float> priorities;
    int i = 0;
    for(const auto &queueFamily : queueFamilies) {
        VkBool32 presentSupport = (surface == nullptr);
        if(presentSupport == false) {
            vkGetPhysicalDeviceSurfaceSupportKHR(s_physicalDevice, i, surface, &presentSupport);
        }

        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
            priorities = std::vector<float>(queueFamily.queueCount, 0.0f);
            s_queueFamilyIndex = i;
            break;
        }
        i++;
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = s_queueFamilyIndex;
    queueCreateInfo.queueCount = priorities.size();
    queueCreateInfo.pQueuePriorities = priorities.data();

    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};

    createInfo.pEnabledFeatures = &deviceFeatures;

    VkResult result = vkCreateDevice(s_physicalDevice, &createInfo, nullptr, &s_device);
    if(result != VK_SUCCESS) {
        aError() << "failed to create logical device!";
    }
}

bool WrapperVk::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

#if defined(SHARED_DEFINE)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {

    qDebug() << "validation layer: " << pCallbackData->pMessage;

    return VK_FALSE;
}

void WrapperVk::setupDebugOutput() {
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(s_instance, "vkCreateDebugUtilsMessengerEXT"));

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
    messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerInfo.pfnUserCallback = debugCallback;

    vkCreateDebugUtilsMessengerEXT(s_instance, &messengerInfo, nullptr, &s_debugMessenger);
}

#endif
