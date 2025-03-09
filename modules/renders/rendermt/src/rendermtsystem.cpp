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

#include <pipelinecontext.h>

static int32_t registered = 0;

RenderMtSystem::RenderMtSystem(Engine *engine) :
        RenderSystem(),
        m_engine(engine),
        m_currentView(nullptr),
        m_currentBuffer(nullptr) {

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
        CommandBufferMt *cmd = static_cast<CommandBufferMt *>(context->buffer());

        RenderTargetMt *defaultTarget = static_cast<RenderTargetMt *>(context->defaultTarget());

        defaultTarget->setNativeHandle(m_currentView->currentRenderPassDescriptor());

        cmd->begin(m_currentBuffer);

        RenderSystem::update(world);

        cmd->end();
    }

}

void RenderMtSystem::setCurrentView(MTK::View *view, MTL::CommandBuffer *cmd) {
    m_currentView = view;
    m_currentBuffer = cmd;
}

#if defined(SHARED_DEFINE)
#include "viewdelegate.h"

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
