#include "pluginmanager.h"

#include <QCoreApplication>
#include <QDirIterator>
#include <QDir>
#include <QLibrary>

#include <log.h>
#include <engine.h>
#include <module.h>
#include <system.h>
#include <components/world.h>
#include <systems/rendersystem.h>

#include "projectsettings.h"

#include <bson.h>
#include <json.h>

#include "config.h"

#if defined(Q_OS_MAC)
#define PLUGINS "/../../../plugins"
#else
#define PLUGINS "/plugins"
#endif

const char *gComponents("components");

PluginManager *PluginManager::m_instance = nullptr;

typedef Module *(*ModuleHandler) (Engine *engine);

PluginManager::PluginManager() :
        QAbstractItemModel(),
        m_engine(nullptr),
        m_renderFactory(nullptr) {

    m_renderName = QString("RenderGL"); // Default
    if(qEnvironmentVariableIsSet(qPrintable(gRhi))) {
        m_renderName = qEnvironmentVariable(qPrintable(gRhi));
    }

    m_initialWhiteList << "RenderGL" << "UiKit" << "Media" << "Bullet" << "Angel";
    m_initialWhiteList << "MotionTools" << "ParticleTools" << "PipelineTools" << "QbsTools" << "ShaderTools";
    m_initialWhiteList << "TextEditor" << "TextureTools" << "TiledImporter" << "Timeline";

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
            Plugin plugin = m_plugins.at(index.row());
            switch(index.column()) {
            case PLUGIN_NAME:        return plugin.name;
            case PLUGIN_DESCRIPTION: return plugin.description;
            case PLUGIN_PATH:        return plugin.path;
            case PLUGIN_VERSION:     return plugin.version;
            case PLUGIN_AUTHOR:      return plugin.author;
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
        auto &plugin = m_plugins[index.row()];
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

    syncWhiteList();

#if defined(__WIN32__) || defined(__APPLE__)
    loadPlugin(QCoreApplication::applicationDirPath() + "/uikit-editor" + gShared);
#endif
    rescanPath(QString(QCoreApplication::applicationDirPath() + PLUGINS));
}

bool PluginManager::rescanProject(QString path) {
    m_pluginPath = path;

    return rescanPath(m_pluginPath);
}

bool PluginManager::loadPlugin(const QString &path, bool reload) {
    QLibrary *lib = new QLibrary(path);
    if(lib->load()) {
        ModuleHandler moduleCreate = reinterpret_cast<ModuleHandler>(lib->resolve("moduleCreate"));
        if(moduleCreate) {
            Module *plugin = moduleCreate(m_engine);
            if(plugin) {
                VariantMap metaInfo = Json::load(plugin->metaInfo()).toMap();

                QFileInfo file(path);

                Plugin plug;
                plug.name = metaInfo[MODULE].toString().c_str();
                plug.path = file.filePath();
                plug.version = metaInfo[VERSION].toString().c_str();
                plug.description = metaInfo[DESC].toString().c_str();
                plug.author = metaInfo[AUTHOR].toString().c_str();
                plug.beta = metaInfo[BETA].toBool();

                if(plug.beta) {
                    plug.tags.push_front("Beta");
                }

                if(plug.path.contains(ProjectSettings::instance()->pluginsPath())) {
                    plug.tags.push_back("Project");
                    m_whiteList.push_back(plug.name);
                }

                plug.library = lib;
                plug.module = plugin;
                plug.enabled = m_whiteList.contains(plug.name);

                if(plug.enabled) {
                    for(auto &it : metaInfo["objects"].toMap()) {
                        bool fault = false;
                        if(it.second == "system") {
                            if(!registerSystem(plugin, it.first.c_str())) {
                                fault = true;
                            }
                        } else if(it.second == "render") {
                            if(QString(it.first.c_str()) == m_renderName) {
                                m_renderFactory = plugin;
                                Engine::addModule(plugin);
                            } else {
                                fault = true;
                            }

                        } else {
                            plug.objects.append(qMakePair(it.first.c_str(), it.second.toString().c_str()));
                        }

                        if(fault) {
                            delete plugin;

                            lib->unload();
                            delete lib;
                            return true;
                        }
                    }

                    for(auto &it : metaInfo[gComponents].toList()) {
                        QString type = QString::fromStdString(it.toString());
                        plug.components << type;
                    }

                    if(!plug.components.isEmpty() && reload) {
                        ComponentBackup result;
                        serializeComponents(plug.components, result);
                        deserializeComponents(result);
                    }
                }

                int index = m_plugins.indexOf(plug);
                if(index == -1) {
                    int start = rowCount();
                    beginInsertRows(QModelIndex(), start, start);
                        m_plugins.push_back(plug);
                    endInsertRows();
                } else {
                    m_plugins[index] = plug;
                }
                return true;
            } else {
                aError() << "[PluginManager] Can't create plugin:" << qPrintable(lib->fileName());
            }
        } else {
            aError() << "[PluginManager] Bad plugin:" << qPrintable(lib->fileName());
        }
    } else {
        aError() << "[PluginManager] Can't load plugin:" << qPrintable(lib->fileName());
    }
    delete lib;
    return false;
}

void PluginManager::reloadPlugin(const QString &path) {
    QFileInfo info(path);

    QFileInfo dest = m_pluginPath + QDir::separator() + info.fileName();
    QFileInfo temp = dest.absoluteFilePath() + ".tmp";

    // Rename old version of plugin
    if(dest.exists()) {
        QFile::remove(temp.absoluteFilePath());
        QFile::rename(dest.absoluteFilePath(), temp.absoluteFilePath());
    }

    Plugin *plugin = nullptr;
    for(auto &it : m_plugins) {
        if(it.path == dest.absoluteFilePath()) {
            plugin = &it;
        }
    }

    if(plugin != nullptr) {
        QStringList components;

        VariantMap metaInfo = Json::load(plugin->module->metaInfo()).toMap();
        for(auto &it : metaInfo[gComponents].toList()) {
            components << QString::fromStdString(it.toString());
        }

        ComponentBackup result;
        serializeComponents(components, result);
        // Unload plugin
        delete plugin->module;

        if(plugin->library->unload()) {
            // Copy new plugin
            if(QFile::copy(path, dest.absoluteFilePath()) && loadPlugin(dest.absoluteFilePath(), true)) {
                deserializeComponents(result);
                // Remove old plugin
                if(QFile::remove(temp.absoluteFilePath())) {
                    aInfo() << "Plugin:" << qPrintable(path) << "reloaded";
                    return;
                }
            }
            delete plugin->library;
        } else {
            aError() << "Plugin unload:" << qPrintable(path) << "failed";
        }
    } else { // Just copy and load plugin
        if(QFile::copy(path, dest.absoluteFilePath()) && loadPlugin(dest.absoluteFilePath())) {
            aInfo() << "Plugin:" << qPrintable(dest.absoluteFilePath()) << "simply loaded";
            return;
        }
    }
    // Rename it back
    if(QFile::remove(dest.absoluteFilePath()) && QFile::rename(temp.absoluteFilePath(), dest.absoluteFilePath())) {
        if(loadPlugin(dest.absoluteFilePath())) {
            aInfo() << "Old version of plugin:" << qPrintable(path) << "is loaded";
        } else {
            aError() << "Load of old version of plugin:" << qPrintable(path) << "is failed";
        }
    }
}

bool PluginManager::rescanPath(const QString &path) {
    bool result = true;
    QDirIterator it(path, { QString("*") + gShared }, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        result &= loadPlugin(it.next());
    }
    return result;
}

bool PluginManager::registerSystem(Module *plugin, const char *name) {
    System *system = reinterpret_cast<System *>(plugin->getObject(name));
    if(system) {
        m_systems[QString::fromStdString(system->name())] = system;
    }

    Engine::addModule(plugin);

    return true;
}

void PluginManager::initSystems() {
    foreach(auto it, m_systems) {
        if(it) {
            it->init();
        }
    }
}

void PluginManager::serializeComponents(const QStringList &list, ComponentBackup &backup) {
    for(auto &type : list) {
        for(auto it : m_engine->getAllObjectsByType(type.toStdString())) {
            const Object::ObjectList &children = it->parent()->getChildren();
            auto pos = std::find(children.begin(), children.end(), it);
            int32_t index = distance(children.begin(), pos);

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
    QStringList toRemove;

    auto &plugins = ProjectSettings::instance()->plugins();
    for(auto it = plugins.begin(); it != plugins.end(); ++it) {
        if(it.value().toBool()) {
            if(m_initialWhiteList.contains(it.key())) {
                toRemove.push_back(it.key());
            } else {
                m_whiteList.push_back(it.key());
            }
        } else {
            if(m_initialWhiteList.contains(it.key())) {
                m_whiteList.removeOne(it.key());
            } else {
                toRemove.push_back(it.key());
            }
        }
    }

    if(!toRemove.empty()) {
        for(auto &it : toRemove) {
            plugins.remove(it);
        }
    }
    ProjectSettings::instance()->saveSettings();
}

RenderSystem *PluginManager::createRenderer() const {
    return reinterpret_cast<RenderSystem *>(m_renderFactory->getObject(qPrintable(m_renderName)));
}

QStringList PluginManager::plugins() const {
    QStringList result;

    for(auto &it : m_plugins) {
        result << it.path;
    }

    return result;
}

QStringList PluginManager::extensions(const QString &type) const {
    QStringList result;

    for(auto &it : m_plugins) {
        if(it.enabled) {
            for(auto &object : it.objects) {
                if(object.second == type) {
                    result << object.first;
                }
            }
        }
    }

    return result;
}

void *PluginManager::getPluginObject(const QString &name) {
    for(auto &it : m_plugins) {
        if(it.enabled) {
            for(auto &object : it.objects) {
                if(object.first == name) {
                    return it.module->getObject(qPrintable(name));
                }
            }
        }
    }

    return nullptr;
}

QString PluginManager::getModuleName(const QString &type) const {
    for(auto &it : m_plugins) {
        if(it.components.indexOf(type) != -1) {
            return it.name;
        }
    }

    return QString();
}
