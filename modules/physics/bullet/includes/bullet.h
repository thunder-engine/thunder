#ifndef BULLET_H
#define BULLET_H

#include <module.h>

class BulletSystem;

class Bullet : public Module {
public:
    Bullet                      (Engine *engine);

    ~Bullet                     ();

    const char                 *description             () const;

    const char                 *version                 () const;

    uint8_t                     types                   () const;

    System                    *system                  ();

    IConverter                 *converter               ();

protected:
    Engine                     *m_pEngine;

    BulletSystem                *m_pSystem;
};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // BULLET_H
