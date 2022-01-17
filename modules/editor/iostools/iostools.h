#ifndef IOSTOOLS_H
#define IOSTOOLS_H

#include <module.h>

class IosTools : public Module {
public:
    IosTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // IOSTOOLS_H
