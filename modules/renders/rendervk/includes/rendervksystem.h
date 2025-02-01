#ifndef RENDERVKSYSTEM_H
#define RENDERVKSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

#include "surfacevk.h"

class Engine;

class RenderVkSystem : public RenderSystem {
public:
    RenderVkSystem(Engine *engine);
    ~RenderVkSystem();

    bool init() override;

    void update(World *world) override;

    void setCurrentSurface(SurfaceVk &surface);

    static int32_t swapChainImageCount();

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow() override;

#endif

private:
    Engine *m_engine;

    SurfaceVk *m_currentSurface;

};

#endif // RENDERVKSYSTEM_H
