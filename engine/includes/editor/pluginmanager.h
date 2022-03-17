#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QAbstractItemModel>
#include <QSet>

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

typedef QHash<Object *, ByteArray> ComponentMap;

class ENGINE_EXPORT PluginManager : public QAbstractItemModel {
    Q_OBJECT

public:
    enum {
        PLUGIN_NAME,
        PLUGIN_DESCRIPTION,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_PATH,
        PLUGIN_LAST
    };

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

    QStringList plugins() const;

    QStringList extensions(const QString &type) const;

    void *getPluginObject(const QString &name);

    QString getModuleName(const QString &type) const;

signals:
    void pluginReloaded();

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

    bool registerRender(Module *plugin, const char *name);

    void serializeComponents(const QStringList &list, ComponentMap &map);

    void deserializeComponents(const ComponentMap &map);

private:
    typedef QMap<QString, System *> SystemsMap;

    struct Plugin {
        bool operator== (const Plugin &left) {
            return path == left.path;
        }

        QString name;

        QString version;

        QString description;

        QString author;

        QString documentation;

        QString url;

        QString path;

        QString baseName;

        QStringList components;

        QList<QPair<QString, QString>> objects;

        QLibrary *library;

        Module *module;
    };

    Engine *m_pEngine;

    RenderSystem *m_pRender;

    QString m_PluginPath;

    SystemsMap m_Systems;

    QList<Scene *> m_Scenes;

    QList<Plugin> m_Plugins;
};

#endif // PLUGINMANAGER_H
