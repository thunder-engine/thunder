#ifndef RENDERGL_H
#define RENDERGL_H

#include <module.h>

class RenderGL : public Module {
public:
    RenderGL(Engine *engine);

    ~RenderGL();

    const char *metaInfo() const override;

    System *system(const char *name) override;

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
