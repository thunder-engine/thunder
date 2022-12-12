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

    ByteArray renderOffscreen(World *world, int width, int height) override;
#endif

private:
    Engine *m_engine;

};

#endif // RENDERGLSYSTEM_H
