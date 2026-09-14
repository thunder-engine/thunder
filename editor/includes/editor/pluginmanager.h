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

#include <QAbstractItemModel>

#include <stdint.h>

#include <editor.h>

class QLibrary;

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

class EDITOR_EXPORT PluginManager : public QAbstractItemModel {
    Q_OBJECT

public:
    enum {
        PLUGIN_NAME,
        PLUGIN_DESCRIPTION,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_PATH,
        PLUGIN_LAST,
        PLUGIN_ENABLED,
        PLUGIN_TAGS,
        PLUGIN_BETA
    };

public:
    PluginManager();
    ~PluginManager();

    void init(Engine *engine);

    bool rescanProject(const TString &path);

    bool loadPlugin(const TString &path, bool reload = false);

    void initSystems();

    bool rescanPath(const TString &path);

    StringList plugins() const;

    StringList dependencies(const TString &type, const StringList &modules = StringList()) const;

    StringList extensions(const TString &type) const;

    void *getPluginObject(const TString &name);

    TString getModuleName(const TString &type) const;

    void syncWhiteList();

signals:
    void pluginReloaded();
    void listChanged();

public slots:
    void reloadPlugin(const TString &path);

private:
    int columnCount(const QModelIndex &) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

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

        QStringList tags;

        std::map<TString, TString> dependencies;

        std::map<TString, TString> objects;

        QLibrary *library;

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

    static PluginManager *m_instance;

    Engine *m_engine;

    Module *m_renderFactory;

};

#endif // PLUGINMANAGER_H
