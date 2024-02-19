#ifndef UIKIT_H
#define UIKIT_H

#include <module.h>

class UiSystem;

class UiKit : public Module {
public:
    UiKit(Engine *engine);
    ~UiKit();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

private:
    UiSystem *m_system;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // UIKIT_H
