#include "rendervksystem.h"

#include <components/camera.h>
#include <components/scene.h>

#include <resources/pipeline.h>

#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "commandbuffervk.h"

#include <log.h>

#if defined(NEXT_SHARED)
#include "editor/vulkanwindow.h"

#include <QLoggingCategory>

QVulkanInstance s_Instance;

Q_LOGGING_CATEGORY(lcVk, "qt.vulkan")
#endif

#define MAX_RESOLUTION 8192

VkDevice s_currentDevice;
VkPhysicalDevice s_currentPhysicalDevice;
VkCommandPool s_currentCommandPool;
VkQueue s_currentQueue;
VkRenderPass s_currentRenderPass;
VkCommandBuffer s_currentCommandBuffer;
VkImage s_currentColorImage;
VkImage s_currentDepthImage;

int32_t s_swapChainImageCount;
int32_t s_currentSwapChainImageIndex;

RenderVkSystem::RenderVkSystem(Engine *engine) :
        RenderSystem(),
        m_pEngine(engine),
        m_registered(false) {
    PROFILE_FUNCTION();

}

RenderVkSystem::~RenderVkSystem() {
    PROFILE_FUNCTION();

}

void RenderVkSystem::registerClasses() {
    RenderSystem::registerClasses();

    System *system = m_pEngine->resourceSystem();

    TextureVk::registerClassFactory(system);
    RenderTargetVk::registerClassFactory(system);
    MaterialVk::registerClassFactory(system);
    MeshVk::registerClassFactory(system);

    CommandBufferVk::registerClassFactory(m_pEngine);

    m_registered = true;
}

void RenderVkSystem::unregisterClasses() {
    if(!m_registered) {
        return;
    }

    RenderSystem::unregisterClasses();

    System *system = m_pEngine->resourceSystem();

    TextureVk::unregisterClassFactory(system);
    RenderTargetVk::unregisterClassFactory(system);
    MaterialVk::unregisterClassFactory(system);
    MeshVk::unregisterClassFactory(system);

    CommandBufferVk::unregisterClassFactory(m_pEngine);
}

const char *RenderVkSystem::name() const {
    return "RenderVK";
}
/*!
    Initialization of render.
*/
bool RenderVkSystem::init() {
    PROFILE_FUNCTION();

#if defined(NEXT_SHARED)
    QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));

    s_Instance.setLayers(QByteArrayList()
                        << "VK_LAYER_LUNARG_standard_validation");

    if(!s_Instance.create()) {
        qFatal("Failed to create Vulkan instance: %d", s_Instance.errorCode());
    }
#endif

    //texture = MIN(texture, MAX_RESOLUTION);
    //setAtlasPageSize(texture, texture);

    CommandBufferVk::setInited();

    return true;
}
/*!
    Main drawing procedure.
*/
void RenderVkSystem::update(Scene *scene) {
    PROFILE_FUNCTION();

    Camera *camera = Camera::current();
    if(camera && CommandBufferVk::isInited()) {
        Pipeline *pipe = camera->pipeline();
        CommandBufferVk *cmd = static_cast<CommandBufferVk *>(pipe->buffer());
        cmd->begin(s_currentCommandBuffer, s_currentColorImage, s_currentDepthImage, s_currentSwapChainImageIndex);
        pipe->setTarget(0);
        RenderSystem::update(scene);
    }
/*
    VkPhysicalDevice device = RenderVkSystem::currentPhysicalDevice();
    VkPhysicalDeviceProperties poperties;
    vkGetPhysicalDeviceProperties(device, &poperties);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device, format, &props);
    const bool canSampleLinear = (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    const bool canSampleOptimal = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
*/
}

void RenderVkSystem::setCurrentDevice(VkDevice device) {
    s_currentDevice = device;
}

VkDevice RenderVkSystem::currentDevice() {
    return s_currentDevice;
}

void RenderVkSystem::setCurrentPhysicalDevice(VkPhysicalDevice device) {
    s_currentPhysicalDevice = device;
}

VkPhysicalDevice RenderVkSystem::currentPhysicalDevice() {
    return s_currentPhysicalDevice;
}

void RenderVkSystem::setCurrentCommandPool(VkCommandPool pool) {
    s_currentCommandPool = pool;
}

VkCommandPool RenderVkSystem::currentCommandPool() {
    return s_currentCommandPool;
}

void RenderVkSystem::setCurrentQueue(VkQueue queue) {
    s_currentQueue = queue;
}

VkQueue RenderVkSystem::currentQueue() {
    return s_currentQueue;
}

void RenderVkSystem::setCurrentRenderPass(VkRenderPass pass) {
    s_currentRenderPass = pass;
}

VkRenderPass RenderVkSystem::currentRenderPass() {
    return s_currentRenderPass;
}

void RenderVkSystem::setCommandBuffer(VkCommandBuffer buffer) {
    s_currentCommandBuffer = buffer;
}

void RenderVkSystem::setCurrentColorImage(VkImage image) {
    s_currentColorImage = image;
}

void RenderVkSystem::setCurrentDepthImage(VkImage image) {
    s_currentDepthImage = image;
}

void RenderVkSystem::setSwapChainImageCount(int32_t count) {
    s_swapChainImageCount = count;
}

int32_t RenderVkSystem::swapChainImageCount() {
    return s_swapChainImageCount;
}

void RenderVkSystem::setCurrentSwapChainImageIndex(int32_t index) {
    s_currentSwapChainImageIndex = index;
}

#if defined(NEXT_SHARED)
QWindow *RenderVkSystem::createRhiWindow() const {
    VulkanWindow *window = new VulkanWindow();
    window->setVulkanInstance(&s_Instance);
    RenderVkSystem::setCurrentPhysicalDevice(window->physicalDevice());
    return window;
}
#endif
