#ifndef BULLET_H
#define BULLET_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef BULLET_LIBRARY
        #define BULLET_EXPORT __declspec(dllexport)
    #else
        #define BULLET_EXPORT __declspec(dllimport)
    #endif
#else
    #define BULLET_EXPORT
#endif

class BulletSystem;

class Bullet : public Module {
public:
    Bullet(Engine *engine);
    ~Bullet();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

protected:
    BulletSystem *m_pSystem;

};

#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // BULLET_H
