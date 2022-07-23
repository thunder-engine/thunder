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

    void update(SceneGraph *scene) override;

#ifdef SHARED_DEFINE
    QWindow *createRhiWindow() const override;

    ByteArray renderOffscreen(SceneGraph *scene, int width, int height) override;
#endif

private:
    Engine *m_pEngine;

};

#endif // RENDERGLSYSTEM_H
