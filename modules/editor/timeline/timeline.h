#ifndef TIMELINE_H
#define TIMELINE_H

#include <module.h>

class TimeLine : public Module {
public:
    TimeLine(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // TIMELINE_H
