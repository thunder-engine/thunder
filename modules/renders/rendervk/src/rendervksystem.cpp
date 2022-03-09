#include "rendervksystem.h"

#include <components/camera.h>
#include <components/scene.h>

#include <resources/pipeline.h>

#include "resources/texturevk.h"
#include "resources/rendertargetvk.h"

#include "commandbuffervk.h"

#include <log.h>

#if defined(SHARED_DEFINE)
#include "editor/vulkanwindow.h"

QVulkanInstance s_Instance;

#endif

#define MAX_RESOLUTION 8192

VkDevice s_currentDevice;
VkPhysicalDevice s_currentPhysicalDevice;
VkCommandPool s_currentCommandPool;
VkQueue s_currentQueue;
VkRenderPass s_currentRenderPass;
VkFramebuffer s_frameBuffer;
VkCommandBuffer s_currentCommandBuffer;
VkImage s_currentColorImage;
VkImage s_currentDepthImage;

int32_t s_swapChainImageCount;
int32_t s_currentSwapChainImageIndex;

uint32_t s_width;
uint32_t s_height;

RenderVkSystem::RenderVkSystem(Engine *engine) :
        RenderSystem(),
        m_pEngine(engine),
        m_registered(false) {

    PROFILE_FUNCTION();

}

RenderVkSystem::~RenderVkSystem() {
    PROFILE_FUNCTION();

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

    System *system = m_pEngine->resourceSystem();

    TextureVk::registerClassFactory(system);
    RenderTargetVk::registerClassFactory(system);
    MaterialVk::registerClassFactory(system);
    MeshVk::registerClassFactory(system);

    CommandBufferVk::registerClassFactory(m_pEngine);

#if defined(SHARED_DEFINE)
    s_Instance.setLayers({"VK_LAYER_KHRONOS_validation"});
    s_Instance.setExtensions({"VK_EXT_debug_utils"});

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
        cmd->begin(s_currentCommandBuffer, s_currentSwapChainImageIndex);

        static_cast<RenderTargetVk *>(pipe->defaultTarget())->setNativeHandle(s_currentRenderPass, s_frameBuffer, s_width, s_height);

        RenderSystem::update(scene);

        cmd->end();
    }
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

void RenderVkSystem::setCurrentFrameBuffer(VkFramebuffer buffer) {
    s_frameBuffer = buffer;
}

void RenderVkSystem::setCommandBuffer(VkCommandBuffer buffer) {
    s_currentCommandBuffer = buffer;
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

void RenderVkSystem::setWindowSize(uint32_t width, uint32_t height) {
    s_width = width;
    s_height = height;
}

#if defined(SHARED_DEFINE)
QWindow *RenderVkSystem::createRhiWindow() const {
    VulkanWindow *window = new VulkanWindow();
    window->setVulkanInstance(&s_Instance);
    RenderVkSystem::setCurrentPhysicalDevice(window->physicalDevice());
    return window;
}

vector<uint8_t> RenderVkSystem::renderOffscreen(Scene *scene, int width, int height) {
    static RenderTarget *target = nullptr;
    if(target == nullptr) {
        target = Engine::objectCreate<RenderTarget>();

        Texture *color = Engine::objectCreate<Texture>();
        color->setFormat(Texture::RGBA8);
        color->setWidth(width);
        color->setHeight(height);

        Texture *depth = Engine::objectCreate<Texture>();
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
        depth->setWidth(width);
        depth->setHeight(height);

        target->setColorAttachment(0, color);
        target->setDepthAttachment(depth);
    }

    Camera *camera = Camera::current();
    if(camera) {
        Pipeline *pipe = camera->pipeline();
        pipe->resize(width, height);

        /// \todo Bind render target
    }

    vector<uint8_t> result;// = RenderSystem::renderOffscreen(scene, width, height);

    result.resize(width * height * 4);
    //glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, result.data());

    return result;
}
#endif
