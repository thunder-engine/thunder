#ifndef BUILDTOOLS_H
#define BUILDTOOLS_H

#include <module.h>

class BuildTools : public Module {
public:
    BuildTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // BUILDTOOLS_H
