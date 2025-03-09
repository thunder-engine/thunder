#ifndef RENDERMT_H
#define RENDERMT_H

#include <module.h>

class RenderMT : public Module {
public:
    RenderMT(Engine *engine);

    ~RenderMT();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // RENDERMT_H
