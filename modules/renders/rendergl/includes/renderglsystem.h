#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <system.h>

class APipeline;

class RenderGLSystem : public ISystem {
public:
    RenderGLSystem              ();
    ~RenderGLSystem             ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene *scene);

    bool                        isThreadSafe                () const;

};

#endif // RENDERGLSYSTEM_H
