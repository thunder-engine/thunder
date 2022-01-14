#ifndef TEXTURETOOLS_H
#define TEXTURETOOLS_H

#include <module.h>

class TextureTools : public Module {
public:
    TextureTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // TEXTURETOOLS_H
