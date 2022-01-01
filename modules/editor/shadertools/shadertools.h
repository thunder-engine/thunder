#ifndef SHADERTOOLS_H
#define SHADERTOOLS_H

#include <module.h>

class ShaderTools : public Module {
public:
    ShaderTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // SHADERTOOLS_H
