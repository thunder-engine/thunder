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
#include "rendermtsystem.h"

#include <components/camera.h>
#include <components/world.h>

#include "resources/meshmt.h"
#include "resources/texturemt.h"
#include "resources/materialmt.h"
#include "resources/rendertargetmt.h"
#include "resources/computebuffermt.h"
#include "resources/computeshadermt.h"

#include "systems/resourcesystem.h"

#include "commandbuffermt.h"
#include "wrappermt.h"
#include "viewdelegate.h"

#include <pipelinecontext.h>

static int32_t registered = 0;

RenderMtSystem::RenderMtSystem(Engine *engine) :
        RenderSystem(),
        m_engine(engine),
        m_currentView(nullptr) {

    PROFILE_FUNCTION();

    if(registered == 0) {
        ResourceSystem *system = Engine::resourceSystem();

        TextureMt::registerClassFactory(system);
        RenderTargetMt::registerClassFactory(system);
        MaterialMt::registerClassFactory(system);
        MeshMt::registerClassFactory(system);
        ComputeBufferMt::registerClassFactory(system);
        ComputeShaderMt::registerClassFactory(system);

        CommandBufferMt::registerClassFactory(system);
    }
    ++registered;
}

RenderMtSystem::~RenderMtSystem() {
    PROFILE_FUNCTION();

    --registered;
    if(registered == 0) {
        ResourceSystem *system = Engine::resourceSystem();

        TextureMt::unregisterClassFactory(system);
        RenderTargetMt::unregisterClassFactory(system);
        MaterialMt::unregisterClassFactory(system);
        MeshMt::unregisterClassFactory(system);
        ComputeBufferMt::unregisterClassFactory(system);
        ComputeShaderMt::unregisterClassFactory(system);

        CommandBufferMt::unregisterClassFactory(system);
    }

    setName("RenderMT");
}
/*!
    Initialization of render.
*/
bool RenderMtSystem::init() {
    PROFILE_FUNCTION();

    bool result = RenderSystem::init();

    CommandBufferMt::setInited();

    return result;
}
/*!
    Main drawing procedure.
*/
void RenderMtSystem::update(World *world) {
    PROFILE_FUNCTION();

    PipelineContext *context = pipelineContext();
    if(context && CommandBufferMt::isInited()) {
        NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
        MTL::CommandBuffer *cmdMt = WrapperMt::queue()->commandBuffer();
        
        MTK::View *view = reinterpret_cast<MTK::View *>(RenderSystem::windowHandle());
        if(view) {
            setCurrentView(view);
        }

        RenderTargetMt *defaultTarget = static_cast<RenderTargetMt *>(context->defaultTarget());
        defaultTarget->setNativeHandle(m_currentView->currentRenderPassDescriptor());

        CommandBufferMt *cmd = static_cast<CommandBufferMt *>(context->buffer());
        cmd->begin(cmdMt);
        RenderSystem::update(world);
        cmd->end();
        
        CA::MetalDrawable *drawable = m_currentView->currentDrawable();
        if(drawable) {
            cmdMt->presentDrawable(drawable);
            cmdMt->commit();
        }

        pool->release();
    }
}

void RenderMtSystem::setCurrentView(MTK::View *view) {
    m_currentView = view;
}

#if defined(SHARED_DEFINE)
#include <QWindow>
#include <QVariant>

QWindow *RenderMtSystem::createRhiWindow(Viewport *viewport) {
    CGRect frame = { {0.0, 0.0}, {100.0, 100.0} };

    MTL::Device *device = WrapperMt::device();
    MTK::View *view = MTK::View::alloc()->init(frame, device);

    view->setFramebufferOnly(false);
    view->setDelegate(new ViewDelegate(this, viewport));

    return QWindow::fromWinId((WId)view);
}

#endif
