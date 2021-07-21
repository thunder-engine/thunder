#ifndef RENDERVK_H
#define RENDERVK_H

#include <module.h>

class RenderVK : public Module {
public:
    RenderVK(Engine *engine);

    ~RenderVK();

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
#endif // RENDERVK_H
