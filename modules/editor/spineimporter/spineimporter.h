#ifndef SPINEIMPORTER_H
#define SPINEIMPORTER_H

#include <module.h>

class SpineImporter : public Module {
public:
    SpineImporter(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // SPINEIMPORTER_H
