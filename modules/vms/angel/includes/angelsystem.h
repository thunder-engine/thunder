#ifndef ANGELSYSTEM_H
#define ANGELSYSTEM_H

#include <system.h>

class asIScriptEngine;
class asIScriptModule;
class asIScriptContext;
class asIScriptFunction;
class asIScriptObject;

struct asSMessageInfo;

class MetaObject;
class Engine;

class AngelSystem : public System {
public:
    AngelSystem (Engine *engine);
    ~AngelSystem ();

    bool init ();

    const char *name () const;

    void update (Scene *);

    bool isThreadSafe () const;

    void reload ();

    void registerClasses (asIScriptEngine *engine);

    void *execute (asIScriptObject *object, asIScriptFunction *func);

    asIScriptModule *module () const;

    asIScriptContext *context() const;

protected:
    void registerMetaType (asIScriptEngine *engine, const MetaType::Table &table);
    void registerMetaObject (asIScriptEngine *engine, const string &name, const MetaObject *meta);

    static void messageCallback (const asSMessageInfo *msg, void *param);

    asIScriptEngine *m_pScriptEngine;

    asIScriptModule *m_pScriptModule;

    asIScriptContext *m_pContext;

    bool m_Inited;
};

#endif // ANGELSYSTEM_H
