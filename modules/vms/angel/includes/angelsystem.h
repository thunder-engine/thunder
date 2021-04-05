#ifndef ANGELSYSTEM_H
#define ANGELSYSTEM_H

#include <system.h>

class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptFunction;
class asIScriptObject;
class asITypeInfo;

struct asSMessageInfo;

class MetaObject;
class Engine;

class AngelScript;

class AngelSystem : public System {
public:
    AngelSystem(Engine *engine);
    ~AngelSystem();

    bool init();

    const char *name() const;

    void update(Scene *);

    int threadPolicy() const;

    void reload();

    void registerClasses(asIScriptEngine *engine);

    void *execute(asIScriptObject *object, asIScriptFunction *func);

    asIScriptModule *module() const;

    asIScriptContext *context() const;

protected:
    bool isBehaviour(asITypeInfo *info) const;

    void unload();

    void bindMetaType(asIScriptEngine *engine, const MetaType::Table &table);
    void bindMetaObject(asIScriptEngine *engine, const string &name, const MetaObject *meta);

    static void messageCallback(const asSMessageInfo *msg, void *param);

    asIScriptEngine *m_pScriptEngine;

    asIScriptModule *m_pScriptModule;

    asIScriptContext *m_pContext;

    AngelScript *m_pScript;

    bool m_Inited;
};

#endif // ANGELSYSTEM_H
