#ifndef MEDIA_H
#define MEDIA_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef MEDIA_LIBRARY
        #define MEDIA_EXPORT __declspec(dllexport)
    #else
        #define MEDIA_EXPORT __declspec(dllimport)
    #endif
#else
    #define MEDIA_EXPORT
#endif

class Media : public Module {
public:
    Media(Engine *engine);

    ~Media();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

protected:
    System *m_pSystem;
};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // MEDIA_H
