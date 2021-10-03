#ifndef ASSETEDITORSMANAGER_H
#define ASSETEDITORSMANAGER_H

#include <QObject>
#include <QMap>
#include <QFileInfo>
#include <QTimer>
#include <QImage>

#include <engine.h>
#include <module.h>

#include <editor/assetconverter.h>

#include <systems/resourcesystem.h>

class QFileSystemWatcher;
class QAbstractItemModel;

class ProjectManager;

class CodeBuilder;

struct Template {
    Template() :
        type(MetaType::INVALID) {

    }
    Template(const QString &p, const uint32_t t) :
        path(p) {
            type = MetaType::name(t);
            type = type.replace("*", "");
            type = type.trimmed();
    }

    QString path;
    QString type;
};
Q_DECLARE_METATYPE(Template)

typedef QFileInfo FilePath;

class AssetManager : public QObject {
    Q_OBJECT
public:
    typedef QMap<QString, AssetConverter *> ConverterMap;
    typedef QMap<QString, QAbstractItemModel *> ClassMap;
    typedef QMap<QString, AssetConverterSettings *> SettingsMap;

public:
    static AssetManager *instance();
    static void destroy();

    void init(Engine *engine);

    void rescan(bool force);

    QString assetTypeName(const QFileInfo &source);
    QStringList assetTypes() const;

    void removeResource(const QFileInfo &source);
    void renameResource(const QFileInfo &oldName, const QFileInfo &newName);
    void duplicateResource(const QFileInfo &source);

    void makePrefab(const QString &source, const QFileInfo &target);

    bool pushToImport(const QFileInfo &source);
    bool import(const QFileInfo &source, const QFileInfo &target);

    void registerConverter(AssetConverter *converter);

    static void findFreeName(QString &name, const QString &path, const QString &suff = QString());

    string guidToPath(const string &guid);
    string pathToGuid(const string &path);

    QImage icon(const QString &source);
    Actor *createActor(const QString &source);

    QStringList labels() const;

    AssetConverterSettings *fetchSettings(const QFileInfo &source);

    AssetConverter *getConverter(AssetConverterSettings *settings);

    bool isOutdated() const;

    QString artifact() const;
    void setArtifact(const QString &value);

    ConverterMap converters() const;
    ClassMap classMaps() const;

    bool pushToImport(AssetConverterSettings *settings);

public slots:
    void reimport();

    void onBuildSuccessful();

signals:
    void ready();

    void directoryChanged(const QString &path);
    void fileChanged(const QString &path);

    void imported(const QString &path, const QString &type);
    void importStarted(int count, const QString &stage);
    void importFinished();

    void prefabCreated(uint32_t uuid, uint32_t clone);

    void buildSuccessful();

protected slots:
    void onPerform();

    void onFileChanged(const QString &path, bool force = false);

    void onDirectoryChanged(const QString &path, bool force = false);

private:
    AssetManager();
    ~AssetManager();

    static AssetManager *m_pInstance;

protected:
    ConverterMap m_Converters;

    ResourceSystem::DictionaryMap &m_Indices;

    VariantMap m_Paths;
    QStringList m_Labels;

    QFileSystemWatcher *m_pDirWatcher;
    QFileSystemWatcher *m_pFileWatcher;

    QList<AssetConverterSettings *> m_ImportQueue;

    ProjectManager *m_pProjectManager;

    QTimer *m_pTimer;

    Engine *m_pEngine;

    ClassMap m_ClassMaps;
    QList<CodeBuilder *> m_Builders;

    QString m_Artifact;

    SettingsMap m_ConverterSettings;

    QHash<QString, QImage> m_Icons;

protected:
    void cleanupBundle();
    void dumpBundle();

    bool isOutdated(AssetConverterSettings *settings);

    bool convert(AssetConverterSettings *settings);

    QString pathToLocal(const QFileInfo &source);

    void registerAsset(const QFileInfo &source, const QString &guid, const QString &type);
    string unregisterAsset(const string &source);
};

#endif // ASSETEDITORSMANAGER_H
