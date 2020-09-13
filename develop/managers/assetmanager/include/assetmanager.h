#ifndef ASSETEDITORSMANAGER_H
#define ASSETEDITORSMANAGER_H

#include <QObject>
#include <QMap>
#include <QFileInfo>
#include <QTimer>
#include <QImage>

#include <engine.h>
#include <module.h>

#include "converters/converter.h"

#include <systems/resourcesystem.h>

class QFileSystemWatcher;
class QAbstractItemModel;

class ProjectManager;

class IBuilder;

struct Template {
    Template() :
        type(MetaType::INVALID) {

    }
    Template(const QString &p, uint32_t t = MetaType::INVALID) :
        path(p),
        type(t) {

    }

    QString path;
    uint32_t type;
};
Q_DECLARE_METATYPE(Template)

typedef QFileInfo FilePath;

class AssetManager : public QObject {
    Q_OBJECT
public:
    typedef QMap<QString, IConverter *> ConverterMap;
    typedef QMap<QString, QAbstractItemModel *> ClassMap;

public:
    static AssetManager *instance();
    static void destroy();

    void init(Engine *engine);

    void rescan(bool force);

    int32_t resourceType(const QFileInfo &source);
    int32_t assetType(const QString &uuid);

    int32_t toContentType(int32_t type);

    void removeResource(const QFileInfo &source);
    void renameResource(const QFileInfo &oldName, const QFileInfo &newName);
    void duplicateResource(const QFileInfo &source);

    void makePrefab(const QString &source, const QFileInfo &target);

    bool pushToImport(const QFileInfo &source);
    bool import(const QFileInfo &source, const QFileInfo &target);

    void registerConverter(IConverter *converter);

    static void findFreeName(QString &name, const QString &path, const QString &suff = QString());

    string guidToPath(const string &guid);
    string pathToGuid(const string &path);

    QImage icon(const QString &path);
    QString type(const QString &path);

    IConverterSettings *fetchSettings(const QFileInfo &source);

    IConverter *getConverter(IConverterSettings *settings);

    bool isOutdated() const;

    QString artifact() const;
    void setArtifact(const QString &value);

    ConverterMap converters() const;
    ClassMap classMaps() const;

    bool pushToImport(IConverterSettings *settings);

public slots:
    void reimport();

signals:
    void ready();

    void directoryChanged(const QString &path);
    void fileChanged(const QString &path);

    void imported(const QString &path, uint32_t type);
    void importStarted(int count, const QString &stage);
    void importFinished();

    void prefabCreated(uint32_t uuid, uint32_t clone);

protected slots:
    void onPerform();

    void onFileChanged(const QString &path, bool force = false);

    void onDirectoryChanged(const QString &path, bool force = false);

private:
    AssetManager();
    ~AssetManager();

    static AssetManager *m_pInstance;

protected:
    typedef QMap<QString, int32_t> FormatsMap;
    FormatsMap m_Formats;

    typedef QMap<int32_t, int32_t> ContentTypeMap;
    ContentTypeMap m_ContentTypes;

    ConverterMap m_Converters;

    ResourceSystem::DictionaryMap &m_Indices;

    VariantMap m_Paths;
    FormatsMap m_Types;

    QFileSystemWatcher *m_pDirWatcher;
    QFileSystemWatcher *m_pFileWatcher;

    QList<IConverterSettings *> m_ImportQueue;

    ProjectManager *m_pProjectManager;

    QTimer *m_pTimer;

    Engine *m_pEngine;

    ClassMap m_ClassMaps;
    QList<IBuilder *> m_Builders;

    QString m_Artifact;

    QMap<QString, IConverterSettings *> m_ConverterSettings;

protected:
    void cleanupBundle();
    void dumpBundle();

    bool isOutdated(IConverterSettings *settings);

    bool convert(IConverterSettings *settings);

    void registerAsset(const string &source, const string &guid, int type);
    string unregisterAsset(const string &source);
};

#endif // ASSETEDITORSMANAGER_H
