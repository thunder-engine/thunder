#ifndef MODULE_H
#define MODULE_H

#include <engine.h>
#include <file.h>

class System;
class IConverter;
class IAssetEditor;

#if defined(NEXT_SHARED) && defined(_WIN32)
    #define MODULE_EXPORT __declspec(dllexport)
#else
    #define MODULE_EXPORT
#endif


class NEXT_LIBRARY_EXPORT Module {
public:
    enum PluginTypes {
        SYSTEM    =(1<<0),
        EXTENSION =(1<<1),
        CONVERTER =(1<<2),
        EDITOR    =(1<<3)
    };

public:
    Module() {}
    virtual ~Module() {}

    virtual const char *description() const = 0;
    virtual const char *version() const = 0;
    virtual uint8_t types() const = 0;

    virtual System *system() { return nullptr; }

    virtual IConverter *converter() { return nullptr; }

    virtual IAssetEditor *assetEditor() { return nullptr; }

    virtual StringList components() const { return StringList(); }
};

#endif // MODULE_H
