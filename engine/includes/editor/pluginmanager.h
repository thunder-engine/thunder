#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QAbstractItemModel>

#include <stdint.h>
#include <variant.h>

#include <engine.h>
#include <global.h>

class QLibrary;

class Object;
class ObjectSystem;
class Scene;
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

    bool rescanProject(const TString &path);

    bool loadPlugin(const TString &path, bool reload = false);

    void initSystems();

    bool rescanPath(const TString &path);

    RenderSystem *createRenderer() const;

    StringList plugins() const;

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

        std::list<std::pair<TString, TString>> objects;

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
