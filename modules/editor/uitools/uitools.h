#ifndef UITOOLS_H
#define UITOOLS_H

#include <module.h>

class UiTools : public Module {
public:
    UiTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // UITOOLS_H
