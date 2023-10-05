#ifndef MOTIONTOOLS_H
#define MOTIONTOOLS_H

#include <module.h>

class MotionTools : public Module {
public:
    MotionTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // MOTIONTOOLS_H
