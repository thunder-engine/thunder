#ifndef Module_H
#define Module_H

#include <engine.h>
#include <file.h>

class System;
class IConverter;

#if defined(NEXT_SHARED) && defined(_WIN32)
    #define MODULE_EXPORT __declspec(dllexport)
#else
    #define MODULE_EXPORT
#endif


class NEXT_LIBRARY_EXPORT Module {
public:
    enum PluginTypes {
        SYSTEM                      = (1<<0),
        EXTENSION                   = (1<<1),
        CONVERTER                   = (1<<2)
    };

public:
    Module                          () {}
    virtual ~Module                 () {}

    virtual const char             *description             () const = 0;
    virtual const char             *version                 () const = 0;
    virtual uint8_t                 types                   () const = 0;

    virtual System                 *system                  () { return nullptr; }

    virtual IConverter             *converter               () { return nullptr; }

    virtual StringList              components              () const { return StringList(); }
};

#endif // Module_H
