#ifndef TILEDIMPORTER_H
#define TILEDIMPORTER_H

#include <module.h>

class TiledImporter : public Module {
public:
    TiledImporter(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // TILEDIMPORTER_H
