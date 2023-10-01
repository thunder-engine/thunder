#ifndef PIPELINETOOLS_H
#define PIPELINETOOLS_H

#include <module.h>

class PipelineTools : public Module {
public:
    PipelineTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // PIPELINETOOLS_H
