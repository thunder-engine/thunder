#ifndef RENDERGL_H
#define RENDERGL_H

#include <module.h>

class RenderGL : public IModule {
public:
    RenderGL                    (Engine *engine);

    ~RenderGL                   ();

    const char                 *description             () const;

    const char                 *version                 () const;

    uint8_t                     types                   () const;

    ISystem                    *system                  ();

protected:
    Engine                     *m_pEngine;

};

extern "C" {
    MODULE_EXPORT IModule *moduleCreate(Engine *engine);
}

#endif // RENDERGL_H
