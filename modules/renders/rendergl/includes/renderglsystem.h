#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

class Engine;

class RenderGLSystem : public RenderSystem {
public:
    RenderGLSystem();
    ~RenderGLSystem();

    bool init() override;

    void update(World *world) override;

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow(Viewport *viewport) override;
#endif

private:
    int32_t m_target;

};

#endif // RENDERGLSYSTEM_H
