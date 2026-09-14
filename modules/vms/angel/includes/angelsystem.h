/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef ANGELSYSTEM_H
#define ANGELSYSTEM_H

#include <system.h>

#include "components/angelbehaviour.h"

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

    void reset() override;

    void update(World *) override;

    int threadPolicy() const override;

    void reload();
    void unloadAll(bool reload);

    void registerClasses(asIScriptEngine *engine);

    asIScriptObject *createScriptObject(const TString &name);

    void *execute(asIScriptObject *object, asIScriptFunction *func);

    asIScriptContext *context() const;

    MetaObject *getMetaObject(const TString &typeName);

    MetaObject *getMetaObject(asITypeInfo *info);

protected:
    bool isBehaviour(asITypeInfo *info) const;

    void bindMetaType(asIScriptEngine *engine, const MetaType::Table &table);
    void bindMetaObject(asIScriptEngine *engine, const TString &name, const MetaObject *meta);

    MetaType::Table *metaType(const TString &typeName);

    void loadModule(const TString &moduleName, AngelScript *script);
    void unloadModule(const TString &moduleName);
    void processModule(asIScriptModule *module);

    static void messageCallback(const asSMessageInfo *msg, void *param);

    static void bundleUpdated(const TString &path, bool unload, void *ptr);

private:
    std::unordered_map<asITypeInfo *, MetaObject *> m_metaObjects;
    std::unordered_map<TString, MetaType::Table *> m_metaTypes;
    std::unordered_map<TString, asIScriptModule *> m_modules;

    asIScriptEngine *m_scriptEngine;

    asIScriptContext *m_context;

    AngelScript *m_script;

    bool m_inited;

    bool m_generic;

};

#endif // ANGELSYSTEM_H
