#include "rendervksystem.h"

#include <pipelinecontext.h>

#include <systems/resourcesystem.h>

#include "resources/meshvk.h"
#include "resources/texturevk.h"
#include "resources/materialvk.h"
#include "resources/rendertargetvk.h"
#include "resources/computebuffervk.h"
#include "resources/computeshadervk.h"

#include "commandbuffervk.h"
#include "wrappervk.h"

#if defined(SHARED_DEFINE)

#include "editor/vulkanwindow.h"

#include <QVulkanInstance>

QVulkanInstance s_QInstance;

#endif

const int MAX_RESOLUTION = 8192;

static int32_t registered = 0;

RenderVkSystem::RenderVkSystem(Engine *engine) :
        RenderSystem(),
        m_engine(engine),
        m_currentSurface(nullptr) {

    PROFILE_FUNCTION();

    if(registered == 0) {
        WrapperVk::createInstance();
        WrapperVk::selectPhysicalDevice();

        System *system = Engine::resourceSystem();

        TextureVk::registerClassFactory(system);
        RenderTargetVk::registerClassFactory(system);
        MaterialVk::registerClassFactory(system);
        MeshVk::registerClassFactory(system);
        ComputeBufferVk::registerClassFactory(system);
        ComputeShaderVk::registerClassFactory(system);

        CommandBufferVk::registerClassFactory(m_engine);
    }
    ++registered;
}

RenderVkSystem::~RenderVkSystem() {
    PROFILE_FUNCTION();

    --registered;
    if(registered == 0) {
        System *system = Engine::resourceSystem();

        TextureVk::unregisterClassFactory(system);
        RenderTargetVk::unregisterClassFactory(system);
        MaterialVk::unregisterClassFactory(system);
        MeshVk::unregisterClassFactory(system);
        ComputeBufferVk::unregisterClassFactory(system);
        ComputeShaderVk::unregisterClassFactory(system);

        CommandBufferVk::unregisterClassFactory(m_engine);

        WrapperVk::destroyContext();

#if defined(SHARED_DEFINE)
        s_QInstance.destroy();
#endif
    }
}

/*!
    Initialization of render.
*/
bool RenderVkSystem::init() {
    PROFILE_FUNCTION();

    Texture::setMaxTextureSize(MAX_RESOLUTION);

    CommandBufferVk::setInited();

    return RenderSystem::init();
}
/*!
    Main drawing procedure.
*/
void RenderVkSystem::update(World *world) {
    PROFILE_FUNCTION();

    PipelineContext *context = pipelineContext();
    if(context && CommandBufferVk::isInited()) {
        CommandBufferVk *cmd = static_cast<CommandBufferVk *>(context->buffer());

        VkCommandBuffer commadBuffer = VK_NULL_HANDLE;

        if(m_currentSurface) {
            commadBuffer = m_currentSurface->currentCmdBuffer();
            m_currentSurface->setupCurrentTarget(context->defaultTarget());
        }

        if(commadBuffer) {
            cmd->begin(commadBuffer);
            RenderSystem::update(world);
            cmd->end();
        }
    }
}

void RenderVkSystem::setCurrentSurface(SurfaceVk &surface) {
    m_currentSurface = &surface;
}

int32_t RenderVkSystem::swapChainImageCount() {
    return SurfaceVk::swapChainImageCount;
}

#if defined(SHARED_DEFINE)
QWindow *RenderVkSystem::createRhiWindow() {
    ThunderVulkanWindow *window = new ThunderVulkanWindow(this);

    if(!s_QInstance.isValid()) {
        s_QInstance.setVkInstance(WrapperVk::instance());

        if(!s_QInstance.create()) {
            qFatal("Failed to create Vulkan instance: %d", s_QInstance.errorCode());
        }
    }

    window->setVulkanInstance(&s_QInstance);

    return window;
}
#endif
