#ifndef RENDERVK_H
#define RENDERVK_H

#include <module.h>

class RenderVK : public Module {
public:
    RenderVK(Engine *engine);

    ~RenderVK() override;

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // RENDERVK_H
