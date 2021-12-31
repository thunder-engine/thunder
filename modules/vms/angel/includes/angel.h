#ifndef ANGEL_H
#define ANGEL_H

#include <module.h>

class AngelSystem;

class Angel : public Module {
public:
    Angel(Engine *engine);

    ~Angel();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

protected:
    AngelSystem *m_pSystem;

};
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // ANGEL_H
