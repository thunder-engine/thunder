#ifndef MODULE_H
#define MODULE_H

#include <engine.h>
#include <file.h>

class System;
class AssetConverter;
class AssetEditor;

#if defined(NEXT_SHARED) && defined(_WIN32)
    #define MODULE_EXPORT __declspec(dllexport)
#else
    #define MODULE_EXPORT
#endif

class NEXT_LIBRARY_EXPORT Module {
public:
    explicit Module(Engine *engine) {}
    virtual ~Module() {}

    virtual System *system(const char *name) { A_UNUSED(name); return nullptr; }

    virtual const char *metaInfo() const = 0;

#ifdef NEXT_SHARED
    virtual AssetConverter *assetConverter(const char *name) { A_UNUSED(name); return nullptr; }

    virtual AssetEditor *assetEditor(const char *name) { A_UNUSED(name); return nullptr; }
#endif
};

#endif // MODULE_H
