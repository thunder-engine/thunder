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

#include "assetmanager.h"
#include "projectmanager.h"

enum {
    PLUGIN_NAME,
    PLUGIN_DESCRIPTION,
    PLUGIN_VERSION,
    PLUGIN_PATH
};

#define PATH    "path"

#define DESC    "description"
#define VERSION "version"
#define SYSTEMS "systems"
#define EXTENSIONS "extensions"
#define CONVERTERS "converters"

#if defined(Q_OS_MAC)
#define PLUGINS "/../../../plugins"
#else
#define PLUGINS "/plugins"
#endif

typedef Module *(*moduleHandler)  (Engine *engine);

PluginManager *PluginManager::m_pInstance = nullptr;

PluginManager::PluginManager() :
        BaseObjectModel(),
        m_pEngine(nullptr),
        m_pRender(nullptr) {

    m_Suffixes = QStringList() << "*.dll" << "*.dylib" << "*.so";

}

PluginManager::~PluginManager() {
    m_Scenes.clear();

    m_Systems.clear();

    foreach(auto it, m_Extensions) {
        delete it;
    }
    m_Extensions.clear();

    foreach(auto it, m_Libraries) {
        delete it;
    }
    m_Libraries.clear();
}

int PluginManager::columnCount(const QModelIndex &) const {
    return 4;
}

QVariant PluginManager::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case PLUGIN_NAME:       return tr("PlugIn Name");
            case PLUGIN_DESCRIPTION:return tr("Description");
            case PLUGIN_VERSION:    return tr("Version");
            case PLUGIN_PATH:       return tr("Full Path");
            default: break;
        }
    }
    return QVariant();
}

QVariant PluginManager::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item = static_cast<QObject *>(index.internalPointer());
    switch(role) {
        case Qt::DisplayRole: {
            switch(index.column()) {
                case PLUGIN_NAME:        return item->objectName();
                case PLUGIN_DESCRIPTION: return item->property(DESC).toString();
                case PLUGIN_PATH:        return item->property(PATH).toString();
                case PLUGIN_VERSION:     return item->property(VERSION).toString();
                default: break;
            }
        } break;
        default: break;
    }

    return QVariant();
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

void PluginManager::rescan() {
    QString system(QCoreApplication::applicationDirPath() + PLUGINS);

    QStringList list = m_Libraries.keys();
    for(auto &it : list) {
        if(!it.contains(system)) {
            PluginsMap::Iterator ext = m_Extensions.find(it);
            if(ext != m_Extensions.end()) {
                Module *plugin = ext.value();
                delete plugin;
            }
            QLibrary *value = m_Libraries.value(it, nullptr);
            if(value && value->unload()) {
                delete value;
            }
            m_Libraries.remove(it);

            QFileInfo info(it);
            QObject *item = m_rootItem->findChild<QObject *>(info.fileName());
            if(item) {
                delete item;
            }
        }
    }
    rescanPath(ProjectManager::instance()->pluginsPath());
}

bool PluginManager::loadPlugin(const QString &path, bool reload) {
    QLibrary *lib = new QLibrary(path);
    if(lib->load()) {
        moduleHandler moduleCreate = reinterpret_cast<moduleHandler>(lib->resolve("moduleCreate"));
        if(moduleCreate) {
            Module *plugin = moduleCreate(m_pEngine);
            if(plugin) {
                VariantMap metaInfo = Json::load(plugin->metaInfo()).toMap();
                for(auto &it : metaInfo[SYSTEMS].toList()) {
                    if(registerSystem(plugin, it.toString().c_str())) {
                        registerExtensionPlugin(path, plugin);
                    } else {
                        delete plugin;

                        lib->unload();
                        delete lib;
                        return true;
                    }
                }
                if(!metaInfo[EXTENSIONS].toList().empty()) {
                    registerExtensionPlugin(path, plugin);

                    if(reload) {
                        QStringList components;
                        for(auto &it : metaInfo[EXTENSIONS].toList()) {
                            components << QString::fromStdString(it.toString());
                        }

                        ComponentMap result;
                        serializeComponents(components, result);
                        deserializeComponents(result);
                    }
                }
                for(auto &it : metaInfo[CONVERTERS].toList()) {
                     AssetManager::instance()->registerConverter(plugin->assetConverter(it.toString().c_str()));
                }

                m_Libraries[path] = lib;

                int start = rowCount();
                beginInsertRows(QModelIndex(), start, start);
                    QFileInfo file(path);

                    QObject *item = new QObject(m_rootItem);
                    item->setObjectName(file.fileName());
                    item->setProperty(PATH, file.filePath());
                    item->setProperty(VERSION, metaInfo[VERSION].toString().c_str());
                    item->setProperty(DESC, metaInfo[DESC].toString().c_str());
                endInsertRows();
                return true;
            } else {
                Log(Log::ERR) << "[ PluginManager::loadPlugin ] Can't create plugin:" << qPrintable(lib->fileName());
            }
        } else {
            Log(Log::ERR) << "[ PluginManager::loadPlugin ] Bad plugin:" << qPrintable(lib->fileName());
        }
    } else {
        Log(Log::ERR) << "[ PluginManager::loadPlugin ] Can't load plugin:" << qPrintable(lib->fileName());
    }
    delete lib;
    return false;
}

void PluginManager::reloadPlugin(const QString &path) {
    QFileInfo info(path);

    QFileInfo dest = ProjectManager::instance()->pluginsPath() + QDir::separator() + info.fileName();
    QFileInfo temp = dest.absoluteFilePath() + ".tmp";

    // Rename old version of plugin
    if(dest.exists()) {
        QFile::remove(temp.absoluteFilePath());
        QFile::rename(dest.absoluteFilePath(), temp.absoluteFilePath());
    }

    PluginsMap::Iterator ext = m_Extensions.find(dest.absoluteFilePath());
    LibrariesMap::Iterator lib = m_Libraries.find(dest.absoluteFilePath());

    if(ext != m_Extensions.end() && lib != m_Libraries.end()) { // Reload plugin
        Module *plugin = ext.value();
        VariantMap metaInfo = Json::load(plugin->metaInfo()).toMap();

        QStringList components;
        for(auto &it : metaInfo[EXTENSIONS].toList()) {
            components << QString::fromStdString(it.toString());
        }

        ComponentMap result;
        serializeComponents(components, result);
        // Unload plugin
        delete plugin;

        QObject *child = m_rootItem->findChild<QObject *>(info.fileName());
        if(child) {
            child->deleteLater();
        }

        if(lib.value()->unload()) {
            // Copy new plugin
            if(QFile::copy(path, dest.absoluteFilePath()) && loadPlugin(dest.absoluteFilePath(), true)) {
                deserializeComponents(result);
                // Remove old plugin
                if(QFile::remove(temp.absoluteFilePath())) {
                    Log(Log::INF) << "Plugin:" << qPrintable(path) << "reloaded";
                    return;
                }
            }
            delete lib.value();
            m_Extensions.remove(ext.key());
        } else {
            Log(Log::ERR) << "Plugin unload:" << qPrintable(path) << "failed";
        }
    } else { // Just copy and load plugin
        if(QFile::copy(path, dest.absoluteFilePath()) && loadPlugin(dest.absoluteFilePath())) {
            Log(Log::INF) << "Plugin:" << qPrintable(path) << "simply loaded";
            return;
        }
    }
    // Rename it back
    if(QFile::remove(dest.absoluteFilePath()) && QFile::rename(temp.absoluteFilePath(), dest.absoluteFilePath())) {
        if(loadPlugin(dest.absoluteFilePath())) {
            Log(Log::INF) << "Old version of plugin:" << qPrintable(path) << "is loaded";
        } else {
            Log(Log::ERR) << "Load of old version of plugin:" << qPrintable(path) << "is failed";
        }
    }
}

void PluginManager::rescanPath(const QString &path) {
    QDirIterator it(path, m_Suffixes, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        loadPlugin(it.next());
    }
}

bool PluginManager::registerSystem(Module *plugin, const char *name) {
    System *system = plugin->system(name);

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

void PluginManager::registerExtensionPlugin(const QString &path, Module *plugin) {
    m_Extensions[path] = plugin;
    emit updated();
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
        for(Scene *scene : m_Scenes) {
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
