#ifndef RENDERMTSYSTEM_H
#define RENDERMTSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

#include "wrappermt.h"

class Engine;

class RenderMtSystem : public RenderSystem {
public:
    RenderMtSystem(Engine *engine);
    ~RenderMtSystem();

    bool init() override;

    void update(World *world) override;

    void setCurrentView(MTK::View *view, MTL::CommandBuffer *cmd);

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow(Viewport *viewport) override;
#endif

private:
    Engine *m_engine;

    MTK::View *m_currentView;

    MTL::CommandBuffer *m_currentBuffer;

};

#endif // RENDERMTSYSTEM_H
