#ifndef ANGEL_H
#define ANGEL_H

#include <module.h>

class AngelSystem;

class Angel : public Module {
public:
    Angel                       (Engine *engine);

    ~Angel                      ();

    const char                 *description             () const;

    const char                 *version                 () const;

    uint8_t                     types                   () const;

    System                    *system                  ();

    IConverter                 *converter               ();

protected:
    Engine                     *m_pEngine;

    AngelSystem                *m_pSystem;
};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // ANGEL_H
