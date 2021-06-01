#ifndef RENDERGL_H
#define RENDERGL_H

#include <module.h>

class RenderGL : public Module {
public:
    RenderGL(Engine *engine);

    ~RenderGL();

    const char *description() const;

    const char *version() const;

    uint8_t types() const;

    System *system();

protected:
    Engine *m_pEngine;

    System *m_pSystem;
};
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // RENDERGL_H
