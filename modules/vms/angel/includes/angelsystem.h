#ifndef ANGELSYSTEM_H
#define ANGELSYSTEM_H

#include <system.h>

#include "components/angelbehaviour.h"

class asIScriptEngine;
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

    void reset() override;

    void update(World *) override;

    int threadPolicy() const override;

    void reload();

    void registerClasses(asIScriptEngine *engine);

    asIScriptObject *createScriptObject(const TString &name);

    void *execute(asIScriptObject *object, asIScriptFunction *func);

    asIScriptContext *context() const;

    MetaObject *getMetaObject(const TString &typeName);

    MetaObject *getMetaObject(asITypeInfo *info);

protected:
    bool isBehaviour(asITypeInfo *info) const;

    void unloadAll(bool reload);

    void bindMetaType(asIScriptEngine *engine, const MetaType::Table &table);
    void bindMetaObject(asIScriptEngine *engine, const TString &name, const MetaObject *meta);

    MetaType::Table *metaType(const TString &typeName);

    static void messageCallback(const asSMessageInfo *msg, void *param);

private:
    std::unordered_map<asITypeInfo *, MetaObject *> m_metaObjects;
    std::unordered_map<TString, MetaType::Table *> m_metaTypes;

    asIScriptEngine *m_scriptEngine;

    asIScriptContext *m_context;

    AngelScript *m_script;

    bool m_inited;

    bool m_generic;

};

#endif // ANGELSYSTEM_H
