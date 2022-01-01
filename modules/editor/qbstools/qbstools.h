#ifndef QBSTOOLS_H
#define QBSTOOLS_H

#include <module.h>

class QbsTools : public Module {
public:
    QbsTools(Engine *engine);

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // QBSTOOLS_H
