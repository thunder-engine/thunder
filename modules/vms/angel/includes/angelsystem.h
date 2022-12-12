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

    bool init() override;

    void update(World *) override;

    int threadPolicy() const override;

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

    asIScriptEngine *m_scriptEngine;

    asIScriptModule *m_scriptModule;

    asIScriptContext *m_context;

    AngelScript *m_script;

    bool m_inited;
};

#endif // ANGELSYSTEM_H
