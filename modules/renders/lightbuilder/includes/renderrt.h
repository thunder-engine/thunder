#ifndef RENDERRT_H
#define RENDERRT_H

#include <module.h>

class RenderRT : public Module {
public:
    RenderRT(Engine *engine);
    ~RenderRT();

    const char *description() const;

    const char *version() const;

    uint8_t types() const;

    System *system();

protected:
    Engine *m_pEngine;

    System *m_pSystem;
};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // RENDERRT_H
