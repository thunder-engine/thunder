#ifndef RENDERGL_H
#define RENDERGL_H

#include <module.h>

class RenderGL : public Module {
public:
    RenderGL(Engine *engine);

    ~RenderGL();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

protected:
    System *m_pSystem;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // RENDERGL_H
