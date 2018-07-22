#include "../include/pluginmodel.h"

#include <QCoreApplication>
#include <QDirIterator>
#include <QDir>
#include <QLibrary>

#include <log.h>
#include <engine.h>
#include <module.h>
#include <system.h>
#include <components/scene.h>

#include <json.h>

#include <rendergl.h>

#include "assetmanager.h"
#include <projectmanager.h>

enum {
    PLUGIN_NAME,
    PLUGIN_DESCRIPTION,
    PLUGIN_VERSION,
    PLUGIN_PATH
};

#define PATH    "path"
#define DESC    "desc"

#define VERSION "version"

#define PLUGINS "/plugins"

typedef IModule *(*moduleHandler)  (Engine *engine);

PluginModel::PluginModel() :
        BaseObjectModel(),
        m_pEngine(nullptr) {

    m_Suffixes = QStringList() << "*.dll";
}

int PluginModel::columnCount(const QModelIndex &) const {
    return 4;
}

QVariant PluginModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const {
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

QVariant PluginModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }

    QObject *item   = static_cast<QObject *>(index.internalPointer());
    switch(role) {
        case Qt::DisplayRole: {
            switch(index.column()) {
                case PLUGIN_NAME:           return item->objectName();
                case PLUGIN_DESCRIPTION:    return item->property(DESC).toString();
                case PLUGIN_PATH:           return item->property(PATH).toString();
                case PLUGIN_VERSION:        return item->property(VERSION).toString();
                default: break;
            }
        } break;
        default: break;
    }

    return QVariant();
}

void PluginModel::init(Engine *engine) {
    m_pEngine   = engine;
}

void PluginModel::rescan() {
    clear();

    rescanPath(QString(QCoreApplication::applicationDirPath() + PLUGINS));
    rescanPath(ProjectManager::instance()->pluginsPath());
}

bool PluginModel::loadPlugin(const QString &path) {
    QLibrary *lib   = new QLibrary(path);
    if(lib->load()) {
        moduleHandler moduleCreate  = (moduleHandler)lib->resolve("moduleCreate");
        if(moduleCreate) {
            IModule *plugin = moduleCreate(m_pEngine);
            if(plugin) {
                m_Libraries[path]   = lib;

                uint8_t types   = plugin->types();
                if(types & IModule::SYSTEM) {
                    registerSystemPlugin(plugin);
                }
                if(types & IModule::EXTENSION) {
                    registerExtensionPlugin(path, plugin);
                }
                if(types & IModule::CONVERTER) {
                     AssetManager::instance()->registerConverter(plugin->converter());
                }
                int start   = rowCount();
                beginInsertRows(QModelIndex(), start, start);
                    QFileInfo file(path);

                    QObject *item  = new QObject(m_rootItem);
                    item->setObjectName(file.fileName());
                    item->setProperty(PATH, file.filePath());
                    item->setProperty(VERSION, plugin->version());
                    item->setProperty(DESC, plugin->description());
                endInsertRows();
                return true;
            } else {
                Log(Log::ERR) << "[ PluginModel::loadPlugin ] Can't create plugin:" << qPrintable(lib->fileName());
            }
        } else {
            Log(Log::ERR) << "[ PluginModel::loadPlugin ] Bad plugin:" << qPrintable(lib->fileName());
        }
    } else {
        Log(Log::ERR) << "[ PluginModel::loadPlugin ] Can't load plugin:" << qPrintable(lib->fileName());
    }
    delete lib;
    return false;
}

void PluginModel::reloadPlugin(const QString &path) {
    QFileInfo info(path);

    QFileInfo dest  = ProjectManager::instance()->pluginsPath() + QDir::separator() + info.fileName();
    QFileInfo temp  = dest.absoluteFilePath() + ".tmp";

    if(dest.exists()) {
        QFile::rename(dest.absoluteFilePath(), temp.absoluteFilePath());
    }

    PluginsMap::Iterator ext    = m_Extensions.find(dest.absoluteFilePath());
    LibrariesMap::Iterator lib  = m_Libraries.find(dest.absoluteFilePath());
    if(ext != m_Extensions.end() && lib != m_Libraries.end()) { // Reload plugin
        IModule *plugin = ext.value();
        StringList list = plugin->components();
        ComponentMap result;
        for(auto type : list) {
            foreach(Scene *scene, m_Scenes) {
                serializeComponents(scene, type, result);
            }
        }
        // Unload plugin
        delete plugin;
        plugin  = nullptr;
        m_Extensions.remove(ext.key());

        QObject *child  = m_rootItem->findChild<QObject *>(info.fileName());
        if(child) {
            child->deleteLater();
        }

        if(lib.value()->unload()) {
            // Copy new plugin
            if(QFile::copy(path, dest.absoluteFilePath()) && loadPlugin(dest.absoluteFilePath())) {
                // Deserialize saved data for components
                ComponentMap::const_iterator i = result.constBegin();
                while(i != result.constEnd()) {
                    Variant v   = Json::load(i.value().toStdString());
                    Object *object  = Engine::toObject(v);
                    if(object) {
                        object->setParent(i.key());
                    }
                    ++i;
                }

                // Remove old plugin
                if(QFile::remove(temp.absoluteFilePath())) {
                    emit pluginReloaded(dest.absoluteFilePath());
                    Log(Log::INF) << "Plugin:" << qPrintable(path) << "reloaded";
                    return;
                }
            }
        }
    } else { // Just copy and load plugin
        if(QFile::copy(path, dest.absoluteFilePath())) {
            if(loadPlugin(dest.absoluteFilePath())) {
                Log(Log::INF) << "Plugin:" << qPrintable(path) << "simply loaded";
                return;
            }
        }
    }
    // Rename it back
    if(QFile::remove(dest.absoluteFilePath())) {
        QFile::rename(temp.absoluteFilePath(), dest.absoluteFilePath());
    }
    // \todo Try to load it back
    Log(Log::ERR) << "Can't reload plugin:" << qPrintable(path);
    return;
}

void PluginModel::clear() {
    beginRemoveRows(QModelIndex(), 0, rowCount());
    delete m_rootItem;
    m_rootItem  = createRoot();
    endRemoveRows();
}

void PluginModel::rescanPath(const QString &path) {
    QDirIterator it(path, m_Suffixes, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        loadPlugin(it.next());
    }
}

void PluginModel::registerSystemPlugin(IModule *plugin) {
    m_Systems[QString::fromStdString(plugin->system()->name())] = plugin;
}

void PluginModel::registerExtensionPlugin(const QString &path, IModule *plugin) {
    m_Extensions[path]  = plugin;
    emit updated();
}

ISystem *PluginModel::createSystem(const QString &name) {
    QMap<QString, IModule *>::iterator it   = m_Systems.find(name);
    if(it != m_Systems.end()) {
        return it.value()->system();
    }
    return nullptr;
}

void PluginModel::addScene(Scene *scene) {
    m_Scenes.push_back(scene);
}

typedef list<const Object *> ObjectArray;
void enumComponents(const Object *object, const string &type, ObjectArray &list) {
    for(const auto &it : object->getChildren()) {
        if(it->typeName() == type) {
            list.push_back(it);
        } else {
            enumComponents(it, type, list);
        }
    }
}

void PluginModel::serializeComponents(Object *parent, const string &type, ComponentMap &map) {
    ObjectArray array;

    enumComponents(parent, type, array);

    for(auto it : array) {
        Variant v   = Engine::toVariant(it);
        map[it->parent()] = Json::save(v).c_str();
        delete it;
    }
}
