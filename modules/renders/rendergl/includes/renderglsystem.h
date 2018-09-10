#ifndef RENDERGLSYSTEM_H
#define RENDERGLSYSTEM_H

#include <cstdint>

#include <system.h>

class APipeline;

class RenderGLSystem : public ISystem {
public:
    RenderGLSystem              (Engine *engine);
    ~RenderGLSystem             ();

    bool                        init                        ();

    const char                 *name                        () const;

    void                        update                      (Scene &scene, uint32_t resource = 0);

    void                        overrideController          (IController *controller);

    void                        resize                      (uint32_t width, uint32_t height);

protected:
    IController                *m_pController;

};

#endif // RENDERGLSYSTEM_H
