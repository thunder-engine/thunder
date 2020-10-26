#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <system.h>

class Engine;

class RenderGLSystem : public System {
public:
    RenderGLSystem(Engine *engine);
    ~RenderGLSystem();

    bool init();

    const char *name() const;

    void update(Scene *scene);

    int threadPolicy() const;

    Engine *m_pEngine;
};

#endif // RENDERGLSYSTEM_H
