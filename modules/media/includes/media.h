#ifndef MEDIA_H
#define MEDIA_H

#include <module.h>

class Media : public Module {
public:
    Media                       (Engine *engine);

    ~Media                      ();

    const char                 *description             () const;

    const char                 *version                 () const;

    uint8_t                     types                   () const;

    System                     *system                  ();

    IConverter                 *converter               ();

protected:
    Engine                     *m_pEngine;

    System                     *m_pSystem;
};

extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}

#endif // MEDIA_H
