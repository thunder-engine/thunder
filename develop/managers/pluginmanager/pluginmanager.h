#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "baseobjectmodel.h"

#include <stdint.h>
#include <variant.h>

#include <file.h>

class QLibrary;

class Engine;
class Module;
class System;
class RenderSystem;

class Object;
class Scene;

typedef QHash<Object *, ByteArray> ComponentMap;

class PluginManager : public BaseObjectModel {
    Q_OBJECT

public:
    static PluginManager *instance();

    static void destroy();

    void init(Engine *engine);

    void rescan();

    bool loadPlugin(const QString &path, bool reload = false);

    int columnCount(const QModelIndex &) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant data(const QModelIndex &index, int role) const;

    void initSystems();

    void addScene(Scene *scene);

    void rescanPath(const QString &path);

    RenderSystem *render() const;

signals:
    void pluginReloaded();

    void updated();

public slots:
    void reloadPlugin(const QString &path);

private:
    PluginManager();

    ~PluginManager();

    static PluginManager *m_pInstance;

protected:
    bool registerSystem(Module *plugin, const char *name);

    void registerExtensionPlugin(const QString &path, Module *plugin);

    void serializeComponents(const QStringList &list, ComponentMap &map);

    void deserializeComponents(const ComponentMap &map);

private:
    typedef QMap<QString, Module *> PluginsMap;

    typedef QMap<QString, QLibrary *> LibrariesMap;

    Engine *m_pEngine;

    QStringList m_Suffixes;

    QMap<QString, System *> m_Systems;

    PluginsMap m_Extensions;

    LibrariesMap m_Libraries;

    QList<Scene *> m_Scenes;

    RenderSystem *m_pRender;
};

#endif // PLUGINMANAGER_H
