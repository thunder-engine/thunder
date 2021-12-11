#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QAbstractItemModel>

#include <stdint.h>
#include <variant.h>

#include <file.h>
#include <global.h>

class QLibrary;

class Object;
class ObjectSystem;
class Scene;
class Engine;
class Module;
class System;
class RenderSystem;
class AssetConverter;

typedef QHash<Object *, ByteArray> ComponentMap;

typedef QMap<QString, AssetConverter *> ConvertersMap;

class NEXT_LIBRARY_EXPORT PluginManager : public QAbstractItemModel {
    Q_OBJECT

public:
    static PluginManager *instance();

    static void destroy();

    void init(Engine *engine);

    void rescan(QString path);

    bool loadPlugin(const QString &path, bool reload = false);

    void initSystems();

    void addScene(Scene *scene);

    void rescanPath(const QString &path);

    RenderSystem *render() const;

    ConvertersMap converters() const;

    QString getModuleName(const QString &type) const;

    QString getModuleName(const ObjectSystem *system) const;

signals:
    void pluginReloaded();

    void updated();

public slots:
    void reloadPlugin(const QString &path);

private:
    PluginManager();

    ~PluginManager();

    int columnCount(const QModelIndex &) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    static PluginManager *m_pInstance;

protected:
    bool registerSystem(Module *plugin, const char *name);

    void registerExtensionPlugin(const QString &path, Module *plugin);

    void serializeComponents(const QStringList &list, ComponentMap &map);

    void deserializeComponents(const ComponentMap &map);

private:
    typedef QMap<QString, Module *> PluginsMap;

    typedef QMap<QString, QLibrary *> LibrariesMap;

    typedef QMap<QString, System *> SystemsMap;

    typedef QMap<QString, QString> ModulesMap;

    struct Plugin {
        bool operator== (const Plugin &left) {
            return path == left.path;
        }

        QString name;

        QString version;

        QString description;

        QString path;
    };

    Engine *m_pEngine;

    RenderSystem *m_pRender;

    QString m_PluginPath;

    QStringList m_Suffixes;

    SystemsMap m_Systems;

    ConvertersMap m_Converters;

    PluginsMap m_Extensions;

    LibrariesMap m_Libraries;

    ModulesMap m_Modules;

    QList<Scene *> m_Scenes;

    QList<Plugin> m_Plugins;
};

#endif // PLUGINMANAGER_H
