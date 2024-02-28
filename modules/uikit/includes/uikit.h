#ifndef UIKIT_H
#define UIKIT_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef UIKIT_LIBRARY
        #define UIKIT_EXPORT __declspec(dllexport)
    #else
        #define UIKIT_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIKIT_EXPORT
#endif

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
