#ifndef PARTICLETOOLS_H
#define PARTICLETOOLS_H

#include <module.h>

class ParticleTools : public Module {
public:
    ParticleTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // PARTICLETOOLS_H
