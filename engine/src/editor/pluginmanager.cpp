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

const char *gComponents("components");

PluginManager *PluginManager::m_instance = nullptr;

typedef Module *(*ModuleHandler) (Engine *engine);

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
    m_initialWhiteList.push_back("QbsTools");
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

            auto &plugins = ProjectSettings::instance()->plugins();
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

PluginManager *PluginManager::instance() {
    if(m_instance == nullptr) {
        m_instance = new PluginManager;
    }
    return m_instance;
}

void PluginManager::destroy() {
    delete m_instance;
}

void PluginManager::init(Engine *engine) {
    m_engine = engine;

    QString uiKit;

#if defined(Q_OS_UNIX)/* && !defined(Q_OS_MAC)*/
    uiKit = QCoreApplication::applicationDirPath() + "/../lib/uikit-editor." + gShared;
#else
    uiKit = QCoreApplication::applicationDirPath() + "/uikit-editor." + gShared;
#endif

    loadPlugin((QCoreApplication::applicationDirPath() + "/uikit-editor." + gShared).toStdString());
    rescanPath((QCoreApplication::applicationDirPath() + "/plugins").toStdString());
}

bool PluginManager::rescanProject(const TString &path) {
    m_pluginPath = path;

    return rescanPath(m_pluginPath);
}

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

                if(plug.path.contains(ProjectSettings::instance()->pluginsPath().data())) {
                    plug.tags.push_back("Project");
                    m_whiteList.push_back(plug.name);
                }

                plug.library = lib;
                plug.module = plugin;
                plug.enabled = std::find(m_whiteList.begin(), m_whiteList.end(), plug.name) != m_whiteList.end();

                if(plug.enabled) {
                    for(auto &it : metaInfo["objects"].toMap()) {
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
                            plug.objects.push_back(std::make_pair(it.first, it.second.toString()));
                        }

                        if(fault) {
                            delete plugin;

                            lib->unload();
                            delete lib;
                            return true;
                        }
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
                aError() << "[PluginManager] Can't create plugin:" << qPrintable(lib->fileName());
            }
        } else {
            aError() << "[PluginManager] Bad plugin:" << qPrintable(lib->fileName());
        }
    } else {
        aError() << "[PluginManager] Can't load plugin:" << qPrintable(lib->fileName()) << "With error:" << qPrintable(lib->errorString());
    }
    delete lib;
    return false;
}

void PluginManager::reloadPlugin(const TString &path) {
    Url info(path.toStdString());

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
                    aInfo() << "Plugin:" << path << "reloaded";
                    return;
                }
            }
            delete plugin->library;
        } else {
            aError() << "Plugin unload:" << path << "failed";
        }
    } else { // Just copy and load plugin
        if(File::copy(path, dest) && loadPlugin(dest)) {
            aInfo() << "Plugin:" << dest << "simply loaded";
            return;
        }
    }
    // Rename it back
    if(File::remove(dest) && File::rename(temp, dest)) {
        if(loadPlugin(dest)) {
            aInfo() << "Old version of plugin:" << path << "is loaded";
        } else {
            aError() << "Load of old version of plugin:" << path << "is failed";
        }
    }
}

bool PluginManager::rescanPath(const TString &path) {
    bool result = true;
    for(auto &it : File::list(path)) {
        Url url(it);
        if(url.suffix() == gShared) {
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

void PluginManager::initSystems() {
    for(auto it : m_systems) {
        it.second->init();
    }
}

void PluginManager::serializeComponents(const StringList &list, ComponentBackup &backup) {
    for(auto &type : list) {
        for(auto it : m_engine->getAllObjectsByType(type)) {
            const Object::ObjectList &children = it->parent()->getChildren();
            auto pos = std::find(children.begin(), children.end(), it);
            int32_t index = std::distance(children.begin(), pos);

            Variant v = Engine::toVariant(it);
            backup.push_back({ Bson::save(v), it->parent(), index });

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

void PluginManager::syncWhiteList() {
    StringList toRemove;

    auto &plugins = ProjectSettings::instance()->plugins();
    for(auto it : plugins) {
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
    ProjectSettings::instance()->saveSettings();
}

RenderSystem *PluginManager::createRenderer() const {
    return reinterpret_cast<RenderSystem *>(m_renderFactory->getObject(m_renderName.data()));
}

StringList PluginManager::plugins() const {
    StringList result;

    for(auto &it : m_plugins) {
        result.push_back(it.path);
    }

    return result;
}

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

TString PluginManager::getModuleName(const TString &type) const {
    for(auto &it : m_plugins) {
        if(std::find(it.components.begin(), it.components.end(), type) != it.components.end()) {
            return it.name;
        }
    }

    return TString();
}
