#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <system.h>

class Engine;

class RenderGLSystem : public ISystem {
public:
    RenderGLSystem              (Engine *engine);
    ~RenderGLSystem             ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene *scene);

    bool                        isThreadSafe                () const;

    Engine *m_pEngine;
};

#endif // RENDERGLSYSTEM_H
