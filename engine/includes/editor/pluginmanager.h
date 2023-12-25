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

struct ComponentData {
    ByteArray data;
    Object *parent;
    int32_t position;
};
typedef QList<ComponentData> ComponentBackup;

class ENGINE_EXPORT PluginManager : public QAbstractItemModel {
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
    static PluginManager *instance();
    static void destroy();

    void init(Engine *engine);

    bool rescanProject(QString path);

    bool loadPlugin(const QString &path, bool reload = false);

    void initSystems();

    bool rescanPath(const QString &path);

    RenderSystem *createRenderer() const;

    QStringList plugins() const;

    QStringList extensions(const QString &type) const;

    void *getPluginObject(const QString &name);

    QString getModuleName(const QString &type) const;

signals:
    void pluginReloaded();
    void listChanged();

public slots:
    void reloadPlugin(const QString &path);

private:
    PluginManager();

    ~PluginManager();

    int columnCount(const QModelIndex &) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

protected:
    bool registerSystem(Module *plugin, const char *name);

    void serializeComponents(const QStringList &list, ComponentBackup &backup);

    void deserializeComponents(const ComponentBackup &backup);

    void syncWhiteList();

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

        QStringList components;

        QStringList tags;

        QList<QPair<QString, QString>> objects;

        QLibrary *library;

        Module *module;

        bool enabled = true;

        bool beta = false;

    };

    static PluginManager *m_instance;

    Engine *m_engine;

    Module *m_renderFactory;

    QString m_pluginPath;

    QString m_renderName;

    SystemsMap m_systems;

    QList<Plugin> m_plugins;

    QStringList m_initialWhiteList;
    QStringList m_whiteList;

};

#endif // PLUGINMANAGER_H
