#ifndef PLUGINMODEL_H
#define PLUGINMODEL_H

#include "baseobjectmodel.h"

#include <stdint.h>
#include <variant.h>

#include <file.h>

class QLibrary;

class Engine;
class Module;
class System;

class Object;
class Scene;

typedef QMap<Object *, ByteArray> ComponentMap;

class PluginModel : public BaseObjectModel {
    Q_OBJECT

public:
    static PluginModel         *instance            ();

    static void                 destroy             ();

    void                        init                        (Engine *engine);

    void                        rescan                      ();

    bool                        loadPlugin                  (const QString &path, bool reload = false);

    int                         columnCount                 (const QModelIndex &) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    void                        initSystems                 ();

    void                        updateSystems               (Scene *scene);

    void                        updateRender                (Scene *scene);

    void                        addScene                    (Scene *scene);

signals:
    void                        pluginReloaded              ();

    void                        updated                     ();

public slots:
    void                        reloadPlugin                (const QString &path);

private:
    PluginModel                 ();

    ~PluginModel                ();

    static PluginModel         *m_pInstance;

protected:
    void                        rescanPath                  (const QString &path);

    void                        registerSystem              (Module *plugin);

    void                        registerExtensionPlugin     (const QString &path, Module *plugin);

    void                        serializeComponents         (const StringList &list, ComponentMap &map);

    void                        deserializeComponents       (const ComponentMap &map);

private:
    void                        clear                       ();

    typedef QMap<QString, Module *>    PluginsMap;

    typedef QMap<QString, QLibrary *>   LibrariesMap;

    Engine                     *m_pEngine;

    QStringList                 m_Suffixes;

    QMap<QString, System *>    m_Systems;

    PluginsMap                  m_Extensions;

    LibrariesMap                m_Libraries;

    QList<Scene *>              m_Scenes;

    System *                   m_pRender;
};

#endif // PLUGINMODEL_H
