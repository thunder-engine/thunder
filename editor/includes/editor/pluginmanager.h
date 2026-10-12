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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <stdint.h>

#include <editor.h>
#include <object.h>

class Library;

class Object;
class Module;
class System;

class Engine;

struct ComponentData {
    ByteArray data;
    Object *parent;
    int32_t position;
};
typedef std::list<ComponentData> ComponentBackup;

struct PluginInfo {
    TString name;
    TString description;
    TString version;
    TString author;
    TString path;
    StringList tags;
    bool enabled;
    bool beta;
};

class EDITOR_EXPORT PluginManager : public Object {
    A_OBJECT(PluginManager, Object, General)

    A_METHODS(
        A_SIGNAL(PluginManager::pluginReloaded),
        A_SIGNAL(PluginManager::listChanged)
    )

public:
    PluginManager();
    ~PluginManager();

    void init(Engine *engine);

    bool rescanProject(const TString &path);

    bool loadPlugin(const TString &path, bool reload = false);

    void initSystems();

    bool rescanPath(const TString &path);

    StringList plugins() const;

    int pluginCount() const;

    bool pluginInfo(int row, PluginInfo &info) const;

    bool setPluginEnabled(int row, bool enabled);

    StringList dependencies(const TString &type, const StringList &modules = StringList()) const;

    StringList extensions(const TString &type) const;

    void *getPluginObject(const TString &name);

    TString getModuleName(const TString &type) const;

    void syncWhiteList();

    void pluginReloaded();

    void listChanged();

    void reloadPlugin(const TString &path);

protected:
    bool registerSystem(Module *plugin, const char *name);

    void serializeComponents(const StringList &list, ComponentBackup &backup);

    void deserializeComponents(const ComponentBackup &backup);

private:
    struct Plugin {
        bool operator== (const Plugin &left) const {
            return path == left.path;
        }

        TString name;

        TString version;

        TString description;

        TString author;

        TString documentation;

        TString url;

        TString path;

        StringList components;

        StringList tags;

        std::map<TString, TString> dependencies;

        std::map<TString, TString> objects;

        Library *library;

        Module *module;

        bool enabled = true;

        bool beta = false;

    };

    TString m_pluginPath;

    TString m_renderName;

    std::map<TString, System *> m_systems;

    std::list<Plugin> m_plugins;

    StringList m_initialWhiteList;
    StringList m_whiteList;

    static LegacyPluginManager *m_instance;

    Engine *m_engine;

};

#endif // PLUGINMANAGER_H
