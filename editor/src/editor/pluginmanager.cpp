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
#include "pluginmanager.h"

#include <QCoreApplication>
#include <QLibrary>

#include <log.h>
#include <url.h>
#include <engine.h>
#include <module.h>
#include <system.h>

#include <systems/rendersystem.h>

#include "projectsettings.h"

#include <bson.h>
#include <json.h>

#include "config.h"

namespace {
    const char *gComponents("components");
    const char *gDependencies("dependencies");
    const char *gObjects("objects");
    const char *gLabel("[PluginManager]");
}

PluginManager *PluginManager::m_instance = nullptr;

typedef Module *(*ModuleHandler) (Engine *engine);

/*!
    \class PluginManager
    \brief Discovers, loads, and manages editor and engine plugins.
    \inmodule Editor

    PluginManager resolves plugin dependencies and provides access to plugin
    modules, systems, components, and editor extensions.

    \fn void PluginManager::pluginReloaded()

    Emitted after a plugin has been reloaded.

    \fn void PluginManager::listChanged()

    Emitted when the plugin list or plugin state changes.
*/

PluginManager::PluginManager() :
        QAbstractItemModel(),
        m_engine(nullptr),
        m_renderFactory(nullptr) {
#ifdef Q_OS_MACOS
    m_renderName = TString("RenderMT"); // Default
#else
    m_renderName = TString("RenderGL"); // Default
#endif

    if(qEnvironmentVariableIsSet(qPrintable(gRhi))) {
        m_renderName = qEnvironmentVariable(qPrintable(gRhi)).toStdString();
    } else {
        qputenv(qPrintable(gRhi), m_renderName.data());
    }

    m_initialWhiteList.push_back("RenderGL");
    m_initialWhiteList.push_back("RenderVK");
    m_initialWhiteList.push_back("RenderMT");
    m_initialWhiteList.push_back("UiKit");
    m_initialWhiteList.push_back("Media");
    m_initialWhiteList.push_back("Bullet");
    m_initialWhiteList.push_back("Angel");
    m_initialWhiteList.push_back("MotionTools");
    m_initialWhiteList.push_back("ParticleTools");
    m_initialWhiteList.push_back("PipelineTools");
    m_initialWhiteList.push_back("BuildTools");
    m_initialWhiteList.push_back("ShaderTools");
    m_initialWhiteList.push_back("TextEditor");
    m_initialWhiteList.push_back("TextureTools");
    m_initialWhiteList.push_back("TiledImporter");
    m_initialWhiteList.push_back("Timeline");
    m_initialWhiteList.push_back("WebTools");

    m_whiteList = m_initialWhiteList;
}

PluginManager::~PluginManager() {
    m_systems.clear();

    for(auto &it : m_plugins) {
        delete it.module;
        delete it.library;
    }
    m_plugins.clear();
}

int PluginManager::columnCount(const QModelIndex &) const {
    return 1;
}

QVariant PluginManager::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

QVariant PluginManager::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    switch(role) {
        case Qt::DisplayRole: {
            Plugin plugin = *std::next(m_plugins.begin(), index.row());
            switch(index.column()) {
            case PLUGIN_NAME:        return plugin.name.data();
            case PLUGIN_DESCRIPTION: return plugin.description.data();
            case PLUGIN_PATH:        return plugin.path.data();
            case PLUGIN_VERSION:     return plugin.version.data();
            case PLUGIN_AUTHOR:      return plugin.author.data();
            case PLUGIN_ENABLED:     return plugin.enabled;
            case PLUGIN_TAGS:        return plugin.tags;
            default: break;
            }
        } break;
        default: break;
    }

    return QVariant();
}

bool PluginManager::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid()) {
        return QAbstractItemModel::setData(index, value, role);
    }

    switch(index.column()) {
        case PLUGIN_ENABLED: {
            auto &plugin = *std::next(m_plugins.begin(), index.row());
            plugin.enabled = value.toBool();

            auto &plugins = Editor::project()->plugins();
            plugins[plugin.name] = plugin.enabled;

            syncWhiteList();

            emit listChanged();

            return true;
        }
        default: break;
    }

    return false;
}

int PluginManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_plugins.size();
}

QModelIndex PluginManager::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return createIndex(row, column);
}

QModelIndex PluginManager::parent(const QModelIndex &child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

/*!
    Initializes the manager and loads the built-in editor plugins.
*/
void PluginManager::init(Engine *engine) {
    m_engine = engine;

    syncWhiteList();

    loadPlugin((QCoreApplication::applicationDirPath() + "/uikit-editor" + gShared).toStdString());
    rescanPath((QCoreApplication::applicationDirPath() + "/plugins").toStdString());
}

/*!
    Sets the project plugin directory and rescans it for plugins.
*/
bool PluginManager::rescanProject(const TString &path) {
    m_pluginPath = path;

    return rescanPath(m_pluginPath);
}

/*!
    Loads the plugin at \a path. If \a reload is true, preserves its components
    while replacing the loaded plugin.
*/
bool PluginManager::loadPlugin(const TString &path, bool reload) {
    QLibrary *lib = new QLibrary(path.data());
    if(lib->load()) {
        ModuleHandler moduleCreate = reinterpret_cast<ModuleHandler>(lib->resolve("moduleCreate"));
        if(moduleCreate) {
            Module *plugin = moduleCreate(m_engine);
            if(plugin) {
                VariantMap metaInfo = Json::load(plugin->metaInfo()).toMap();

                Plugin plug;
                plug.name = metaInfo[MODULE].toString();
                plug.path = path;
                plug.version = metaInfo[VERSION].toString();
                plug.description = metaInfo[DESC].toString();
                plug.author = metaInfo[AUTHOR].toString();
                plug.beta = metaInfo[BETA].toBool();

                if(plug.beta) {
                    plug.tags.push_front("Beta");
                }

                if(plug.path.contains(Editor::project()->pluginsPath().data())) {
                    plug.tags.push_back("Project");
                    m_whiteList.push_back(plug.name);
                }

                plug.library = lib;
                plug.module = plugin;
                plug.enabled = std::find(m_whiteList.begin(), m_whiteList.end(), plug.name) != m_whiteList.end();

                if(plug.enabled) {
                    for(auto &it : metaInfo[gObjects].toMap()) {
                        bool fault = false;
                        if(it.second == "system") {
                            if(!registerSystem(plugin, it.first.data())) {
                                fault = true;
                            }
                        } else if(it.second == "render") {
                            if(it.first == m_renderName) {
                                m_renderFactory = plugin;
                                Engine::addModule(plugin);
                            } else {
                                fault = true;
                            }

                        } else {
                            plug.objects[it.first] = it.second.toString();
                        }

                        if(fault) {
                            delete plugin;

                            lib->unload();
                            delete lib;
                            return true;
                        }
                    }

                    for(auto &it : metaInfo[gDependencies].toMap()) {
                        plug.dependencies[it.first] = it.second.toString();
                    }

                    for(auto &it : metaInfo[gComponents].toList()) {
                        plug.components.push_back(it.toString());
                    }

                    if(!plug.components.empty() && reload) {
                        ComponentBackup result;
                        serializeComponents(plug.components, result);
                        deserializeComponents(result);
                    }
                }

                auto it = std::find(m_plugins.begin(), m_plugins.end(), plug);
                if(it == m_plugins.end()) {
                    int start = rowCount();
                    beginInsertRows(QModelIndex(), start, start);
                        m_plugins.push_back(plug);
                    endInsertRows();
                } else {
                    int index = std::distance(m_plugins.begin(), it);
                    *std::next(m_plugins.begin(), index) = plug;
                }
                return true;
            } else {
                aError() << gLabel << "Can't create plugin:" << qPrintable(lib->fileName());
            }
        } else {
            aError() << gLabel << "Bad plugin:" << qPrintable(lib->fileName());
        }
    } else {
        aError() << gLabel << "Can't load plugin:" << qPrintable(lib->fileName()) << "With error:" << qPrintable(lib->errorString());
    }
    delete lib;
    return false;
}

/*!
    Replaces the installed project plugin with the plugin at \a path.
*/
void PluginManager::reloadPlugin(const TString &path) {
    Url info(path);

    TString dest(m_pluginPath + "/" + info.name());
    TString temp(dest + ".tmp");

    // Rename old version of plugin
    if(File::exists(dest)) {
        File::remove(temp);
        File::rename(dest, temp);
    }

    Plugin *plugin = nullptr;
    for(auto &it : m_plugins) {
        if(it.path == dest) {
            plugin = &it;
            break;
        }
    }

    if(plugin != nullptr) {
        StringList components;

        VariantMap metaInfo = Json::load(plugin->module->metaInfo()).toMap();
        for(auto &it : metaInfo[gComponents].toList()) {
            components.push_back(it.toString());
        }

        ComponentBackup result;
        serializeComponents(components, result);
        // Unload plugin
        delete plugin->module;

        if(plugin->library->unload()) {
            // Copy new plugin
            if(File::copy(path, dest) && loadPlugin(dest, true)) {
                deserializeComponents(result);
                // Remove old plugin
                if(File::remove(temp)) {
                    aInfo() << gLabel << "Plugin:" << path << "reloaded";
                    return;
                }
            }
            delete plugin->library;
        } else {
            aError() << gLabel << "Plugin unload:" << path << "failed";
        }
    } else { // Just copy and load plugin
        if(File::copy(path, dest) && loadPlugin(dest)) {
            aInfo() << gLabel << "Plugin:" << dest << "loaded";
            return;
        }
    }
    // Rename it back
    if(File::remove(dest) && File::rename(temp, dest)) {
        if(loadPlugin(dest)) {
            aInfo() << gLabel << "Old version of plugin:" << path << "is loaded";
        } else {
            aError() << gLabel << "Load of old version of plugin:" << path << "is failed";
        }
    }
}

/*!
    Scans \a path and loads all shared-library plugins found there.
*/
bool PluginManager::rescanPath(const TString &path) {
    bool result = true;
    for(auto &it : File::list(path)) {
        Url url(it);
        if(TString(".") + url.suffix() == gShared) {
            result &= loadPlugin(it);
        }
    }
    return result;
}

bool PluginManager::registerSystem(Module *plugin, const char *name) {
    System *system = reinterpret_cast<System *>(plugin->getObject(name));
    if(system) {
        m_systems[system->name()] = system;
    }

    Engine::addModule(plugin);

    return true;
}

/*!
    Initializes all systems provided by enabled plugins.
*/
void PluginManager::initSystems() {
    for(auto &it : m_systems) {
        it.second->init();
    }
}

void PluginManager::serializeComponents(const StringList &list, ComponentBackup &backup) {
    for(auto &type : list) {
        for(auto it : m_engine->getAllObjectsByType(type)) {
            Object *parent = it->parent();

            const Object::ObjectList &children = parent->getChildren();
            auto pos = std::find(children.begin(), children.end(), it);
            int32_t index = std::distance(children.begin(), pos);

            Variant v = Engine::toVariant(it);
            backup.push_back({ Bson::save(v), parent, index });

            delete it;
        }
    }
}

void PluginManager::deserializeComponents(const ComponentBackup &backup) {
    for(auto &it : backup) {
        Variant v = Bson::load(it.data);
        Object *object = Engine::toObject(v, it.parent);
        if(object) {
            object->setParent(it.parent, it.position);
        }
    }
    emit pluginReloaded();
}

/*!
    Synchronizes the enabled plugin whitelist with project settings.
*/
void PluginManager::syncWhiteList() {
    StringList toRemove;

    auto &plugins = Editor::project()->plugins();
    for(auto &it : plugins) {
        if(it.second) {
            if(std::find(m_initialWhiteList.begin(), m_initialWhiteList.end(), it.first) != m_initialWhiteList.end()) {
                toRemove.push_back(it.first);
            } else {
                m_whiteList.push_back(it.first);
            }
        } else {
            if(std::find(m_initialWhiteList.begin(), m_initialWhiteList.end(), it.first) != m_initialWhiteList.end()) {
                m_whiteList.remove(it.first);
            } else {
                toRemove.push_back(it.first);
            }
        }
    }

    if(!toRemove.empty()) {
        for(auto &it : toRemove) {
            plugins.erase(it);
        }
    }
    Editor::project()->saveSettings();
}

/*!
    Returns paths of all plugins known to the manager.
*/
StringList PluginManager::plugins() const {
    StringList result;

    for(auto &it : m_plugins) {
        result.push_back(it.path);
    }

    return result;
}

/*!
    Returns dependencies of the specified \a type for the requested modules.
*/
StringList PluginManager::dependencies(const TString &type, const StringList &modules) const {
    StringList result;

    for(auto &plugin : m_plugins) {
        if(plugin.enabled && (modules.empty() || std::find(modules.begin(), modules.end(), plugin.name) != modules.end())) {
            for(auto &dependency : plugin.dependencies) {
                if(dependency.second == type && std::find(result.begin(), result.end(), dependency.first) == result.end()) {
                    result.push_back(dependency.first);
                }
            }
        }
    }

    return result;
}

/*!
    Returns plugin extensions registered for the specified \a type.
*/
StringList PluginManager::extensions(const TString &type) const {
    StringList result;

    for(auto &it : m_plugins) {
        if(it.enabled) {
            for(auto &object : it.objects) {
                if(object.second == type) {
                    result.push_back(object.first);
                }
            }
        }
    }

    return result;
}

/*!
    Creates and returns the plugin object registered under \a name.
*/
void *PluginManager::getPluginObject(const TString &name) {
    for(auto &it : m_plugins) {
        if(it.enabled) {
            for(auto &object : it.objects) {
                if(object.first == name) {
                    return it.module->getObject(name.data());
                }
            }
        }
    }

    return nullptr;
}

/*!
    Returns the plugin module that provides the specified component \a type.
*/
TString PluginManager::getModuleName(const TString &type) const {
    for(auto &it : m_plugins) {
        if(std::find(it.components.begin(), it.components.end(), type) != it.components.end()) {
            return it.name;
        }
    }

    return TString();
}
