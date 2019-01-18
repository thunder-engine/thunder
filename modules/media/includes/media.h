#ifndef MEDIA_H
#define MEDIA_H

#include <module.h>

class Media : public IModule {
public:
    Media                       (Engine *engine);

    ~Media                      ();

    const char                 *description             () const;

    const char                 *version                 () const;

    uint8_t                     types                   () const;

    ISystem                    *system                  ();

    IConverter                 *converter               ();

protected:
    Engine                     *m_pEngine;

};

extern "C" {
    MODULE_EXPORT IModule *moduleCreate(Engine *engine);
}

#endif // MEDIA_H
