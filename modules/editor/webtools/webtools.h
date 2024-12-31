#ifndef WEBTOOLS_H
#define WEBTOOLS_H

#include <module.h>

class WebTools : public Module {
public:
    WebTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // WEBTOOLS_H
