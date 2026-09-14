/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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

RenderVkSystem::RenderVkSystem(Engine *engine) :
        RenderSystem(),
        m_engine(engine),
        m_currentSurface(nullptr) {

    PROFILE_FUNCTION();

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

RenderVkSystem::~RenderVkSystem() {
    PROFILE_FUNCTION();

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
QWindow *RenderVkSystem::createRhiWindow(Viewport *viewport) {
    ThunderVulkanWindow *window = new ThunderVulkanWindow(viewport, this);

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
