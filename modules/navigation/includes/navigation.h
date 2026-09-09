#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef NAVIGATION_LIBRARY
        #define NAVIGATION_EXPORT __declspec(dllexport)
    #else
        #define NAVIGATION_EXPORT __declspec(dllimport)
    #endif
#else
    #define NAVIGATION_EXPORT
#endif

class EditorGadget;

class Navigation : public Module {
public:
    Navigation(Engine *engine);
    ~Navigation();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

protected:
    System *m_system;
    EditorGadget *m_panel;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // NAVIGATION_H
