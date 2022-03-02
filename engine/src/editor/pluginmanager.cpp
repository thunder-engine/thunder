#include "pluginmanager.h"

#include <QCoreApplication>
#include <QDirIterator>
#include <QDir>
#include <QLibrary>

#include <log.h>
#include <engine.h>
#include <module.h>
#include <system.h>
#include <components/scene.h>
#include <systems/rendersystem.h>

#include <bson.h>
#include <json.h>

#if defined(Q_OS_MAC)
#define PLUGINS "/../../../plugins"
#else
#define PLUGINS "/plugins"
#endif

const char *gComponents("components");

QStringList g_Suffixes = { "*.dll", "*.dylib", "*.so" };

typedef Module *(*moduleHandler) (Engine *engine);

PluginManager *PluginManager::m_pInstance = nullptr;

PluginManager::PluginManager() :
        QAbstractItemModel(),
        m_pEngine(nullptr),
        m_pRender(nullptr) {

}

PluginManager::~PluginManager() {
    m_Scenes.clear();

    m_Systems.clear();

    for(auto &it : m_Plugins) {
        delete it.module;
        delete it.library;
    }
    m_Plugins.clear();
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
            Plugin plugin = m_Plugins.at(index.row());
            switch(index.column()) {
                case PLUGIN_NAME:        return plugin.name;
                case PLUGIN_DESCRIPTION: return plugin.description;
                case PLUGIN_PATH:        return plugin.path;
                case PLUGIN_VERSION:     return plugin.version;
                case PLUGIN_AUTHOR:      return plugin.author;
                default: break;
            }
        } break;
        default: break;
    }

    return QVariant();
}

int PluginManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_Plugins.size();
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
    if(!m_pInstance) {
        m_pInstance = new PluginManager;
    }
    return m_pInstance;
}

void PluginManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void PluginManager::init(Engine *engine) {
    m_pEngine = engine;

    rescanPath(QString(QCoreApplication::applicationDirPath() + PLUGINS));
}

void PluginManager::rescan(QString path) {
    m_PluginPath = path;

    rescanPath(m_PluginPath);
}

bool PluginManager::loadPlugin(const QString &path, bool reload) {
    QLibrary *lib = new QLibrary(path);
    if(lib->load()) {
        moduleHandler moduleCreate = reinterpret_cast<moduleHandler>(lib->resolve("moduleCreate"));
        if(moduleCreate) {
            Module *plugin = moduleCreate(m_pEngine);
            if(plugin) {
                VariantMap metaInfo = Json::load(plugin->metaInfo()).toMap();

                QFileInfo file(path);

                Plugin plug;
                plug.name = metaInfo["module"].toString().c_str();
                plug.path = file.filePath();
                plug.version = metaInfo[VERSION].toString().c_str();
                plug.description = metaInfo[DESC].toString().c_str();
                plug.author = metaInfo[AUTHOR].toString().c_str();
                plug.library = lib;
                plug.module = plugin;

                for(auto &it : metaInfo["objects"].toMap()) {
                    if(it.second == "system") {
                        if(!registerSystem(plugin, it.first.c_str())) {
                            delete plugin;

                            lib->unload();
                            delete lib;
                            return true;
                        }
                    } else {
                        plug.objects.append(qMakePair(it.first.c_str(), it.second.toString().c_str()));
                    }
                }

                for(auto &it : metaInfo[gComponents].toList()) {
                    QString type = QString::fromStdString(it.toString());
                    plug.components << type;
                }

                if(!plug.components.isEmpty() && reload) {
                    ComponentMap result;
                    serializeComponents(plug.components, result);
                    deserializeComponents(result);
                }

                int index = m_Plugins.indexOf(plug);
                if(index == -1) {
                    int start = rowCount();
                    beginInsertRows(QModelIndex(), start, start);
                        m_Plugins.push_back(plug);
                    endInsertRows();
                } else {
                    m_Plugins[index] = plug;
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

    QFileInfo dest = m_PluginPath + QDir::separator() + info.fileName();
    QFileInfo temp = dest.absoluteFilePath() + ".tmp";

    // Rename old version of plugin
    if(dest.exists()) {
        QFile::remove(temp.absoluteFilePath());
        QFile::rename(dest.absoluteFilePath(), temp.absoluteFilePath());
    }

    Plugin *plugin = nullptr;
    for(auto &it : m_Plugins) {
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

        ComponentMap result;
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

void PluginManager::rescanPath(const QString &path) {
    QDirIterator it(path, g_Suffixes, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        loadPlugin(it.next());
    }
}

bool PluginManager::registerSystem(Module *plugin, const char *name) {
    System *system = reinterpret_cast<System *>(plugin->getObject(name));
    if(system) {
        RenderSystem *render = dynamic_cast<RenderSystem *>(system);
        if(render) {
            QString renderName("RenderGL");
            if(qEnvironmentVariableIsSet("RENDER")) {
                renderName = qEnvironmentVariable("RENDER");
            }

            if(QString(render->name()) == renderName) {
                m_pRender = render;
            } else {
                return false;
            }
        }

        m_Systems[QString::fromStdString(system->name())] = system;
    }

    Engine::addModule(plugin);

    return true;
}

void PluginManager::initSystems() {
    foreach(auto it, m_Systems) {
        if(it) {
            it->init();
        }
    }
}

void PluginManager::addScene(Scene *scene) {
    m_Scenes.push_back(scene);
}

typedef list<const Object *> ObjectArray;
void enumComponents(const Object *object, const QString &type, ObjectArray &list) {
    for(const auto &it : object->getChildren()) {
        if(it->typeName() == type.toStdString()) {
            list.push_back(it);
        } else {
            enumComponents(it, type, list);
        }
    }
}

void PluginManager::serializeComponents(const QStringList &list, ComponentMap &map) {
    for(auto &type : list) {
        foreach(Scene *scene, m_Scenes) {
            ObjectArray array;

            enumComponents(scene, type, array);

            for(auto it : array) {
                Variant v = Engine::toVariant(it);
                map[it->parent()] = Bson::save(v);
                delete it;
            }
        }
    }
}

void PluginManager::deserializeComponents(const ComponentMap &map) {
    auto it = map.constBegin();
    while(it != map.constEnd()) {
        Variant v = Bson::load(it.value());
        Object *object = Engine::toObject(v);
        if(object) {
            object->setParent(it.key());
        }
        ++it;
    }
    emit pluginReloaded();
}

RenderSystem *PluginManager::render() const {
    return m_pRender;
}

QStringList PluginManager::plugins() const {
    QStringList result;

    for(auto &it : m_Plugins) {
        result << it.path;
    }

    return result;
}

QStringList PluginManager::extensions(const QString &type) const {
    QStringList result;

    for(auto &it : m_Plugins) {
        for(auto &object : it.objects) {
            if(object.second == type) {
                result << object.first;
            }
        }
    }

    return result;
}

void *PluginManager::getPluginObject(const QString &name) {
    for(auto &it : m_Plugins) {
        for(auto &object : it.objects) {
            if(object.first == name) {
                return it.module->getObject(qPrintable(name));
            }
        }
    }

    return nullptr;
}

QString PluginManager::getModuleName(const QString &type) const {
    for(auto &it : m_Plugins) {
        if(it.components.indexOf(type) != -1) {
            return it.name;
        }
    }

    return QString();
}
