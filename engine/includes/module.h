#ifndef IMODULE_H
#define IMODULE_H

#include <engine.h>
#include <file.h>

class ISystem;

class NEXT_LIBRARY_EXPORT IModule {
public:
    enum PluginTypes {
        SYSTEM                      = (1<<0),
        EXTENSION                   = (1<<1)
    };

public:
    IModule                         () {}

    virtual ~IModule                () {}

    virtual const char             *description             () const = 0;
    virtual const char             *version                 () const = 0;
    virtual uint8_t                 types                   () const = 0;

    virtual ISystem                *system                  () { return nullptr; }

    virtual StringList              components              () const { return StringList(); }
};

#endif // IMODULE_H
