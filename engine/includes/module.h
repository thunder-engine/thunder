#ifndef MODULE_H
#define MODULE_H

#include <engine.h>
#include <file.h>

class System;
class AssetConverter;
class AssetEditor;

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #define MODULE_EXPORT __declspec(dllexport)
#else
    #define MODULE_EXPORT
#endif

#define MODULE  "module"
#define DESC    "description"
#define VERSION "version"
#define AUTHOR  "author"

class ENGINE_EXPORT Module {
public:
    explicit Module(Engine *engine) { m_engine = engine; }
    virtual ~Module() {}

    virtual const char *metaInfo() const = 0;

    virtual void *getObject(const char *name) { A_UNUSED(name); return nullptr; }

protected:
    Engine *m_engine;

};

#endif // MODULE_H
