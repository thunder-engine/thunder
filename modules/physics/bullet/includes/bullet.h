#ifndef BULLET_H
#define BULLET_H

#include <module.h>

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
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // BULLET_H
