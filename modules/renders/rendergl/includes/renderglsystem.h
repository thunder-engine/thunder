#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <systems/rendersystem.h>

class Engine;

class RenderGLSystem : public RenderSystem {
public:
    RenderGLSystem(Engine *engine);
    ~RenderGLSystem();

    bool init() override;

    void update(World *world) override;

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow() override;
#endif

private:
    Engine *m_engine;

    int32_t m_target;
};

#endif // RENDERGLSYSTEM_H
