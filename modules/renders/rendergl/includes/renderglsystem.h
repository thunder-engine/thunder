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

    const char *name() const override;

    void update(Scene *scene) override;

    void registerClasses() override;

    void unregisterClasses() override;

#ifdef NEXT_SHARED
    QWindow *createRhiWindow() const override;
#endif

private:
    Engine *m_pEngine;

    bool m_registered;

};

#endif // RENDERGLSYSTEM_H
