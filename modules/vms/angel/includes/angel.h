#ifndef ANGEL_H
#define ANGEL_H

#include <module.h>

class NEXT_LIBRARY_EXPORT Angel : public IModule {
public:
    Angel                       (Engine *engine);

    ~Angel                      ();

    const char                 *description             () const;

    const char                 *version                 () const;

    uint8_t                     types                   () const;

    ISystem                    *system                  ();

    IConverter                 *converter               ();

protected:
    Engine                     *m_pEngine;

};

extern "C" {
    NEXT_LIBRARY_EXPORT IModule *moduleCreate(Engine *engine);
}

#endif // ANGEL_H
