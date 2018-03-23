#ifndef PLUGINMODEL_H
#define PLUGINMODEL_H

#include "baseobjectmodel.h"

#include <stdint.h>
#include <patterns/asingleton.h>

class QLibrary;

class Engine;
class IModule;
class ISystem;

class Object;
class Scene;

typedef QMap<Object *, QString> ComponentMap;

class PluginModel : public BaseObjectModel, public ASingleton<PluginModel> {
    Q_OBJECT

public:
    void                        init                        (Engine *engine);

    void                        rescan                      ();

    bool                        loadPlugin                  (const QString &path);

    int                         columnCount                 (const QModelIndex &) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    ISystem                    *createSystem                (const QString &name);

    void                        addScene                    (Scene *scene);

signals:
    void                        pluginReloaded              (const QString &path);

    void                        updated                     ();

public slots:
    void                        reloadPlugin                (const QString &path);

protected:
    friend class ASingleton<PluginModel>;

    PluginModel                 ();

    void                        rescanPath                  (const QString &path);

    void                        registerSystemPlugin        (IModule *plugin);

    void                        registerExtensionPlugin     (const QString &path, IModule *plugin);

    void                        serializeComponents         (Object *parent, const std::string &type, ComponentMap &map);

private:
    void                        clear                       ();

    typedef QMap<QString, IModule *>   PluginsMap;

    typedef QMap<QString, QLibrary *>   LibrariesMap;

    Engine                     *m_pEngine;

    QStringList                 m_Suffixes;

    PluginsMap                  m_Systems;

    PluginsMap                  m_Extensions;

    LibrariesMap                m_Libraries;

    QList<Scene *>              m_Scenes;
};

#endif // PLUGINMODEL_H
