#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <QObject>
#include <QMap>
#include <QFileInfo>
#include <QTimer>
#include <QImage>
#include <QSet>

#include <engine.h>
#include <module.h>

#include <editor/assetconverter.h>

#include <systems/resourcesystem.h>

class QFileSystemWatcher;
class QAbstractItemModel;

class ProjectSettings;

class CodeBuilder;

class ENGINE_EXPORT AssetManager : public QObject {
    Q_OBJECT

public:
    typedef QMap<QString, AssetConverter *> ConverterMap;
    typedef QMap<QString, QAbstractItemModel *> ClassMap;
    typedef QMap<QString, AssetConverterSettings *> SettingsMap;

public:
    static AssetManager *instance();
    static void destroy();

    void init();

    void rescan(bool force);

    void setNoIcons();

    void rebuild();

    QString assetTypeName(const QFileInfo &source);

    void removeResource(const QFileInfo &source);
    void renameResource(const QFileInfo &oldName, const QFileInfo &newName);
    void duplicateResource(const QFileInfo &source);

    void makePrefab(const QString &source, const QFileInfo &target);

    bool pushToImport(const QFileInfo &source);
    bool import(const QFileInfo &source, const QFileInfo &target);

    void registerConverter(AssetConverter *converter);

    static void findFreeName(QString &name, const QString &path, const QString &suff = QString());

    std::string guidToPath(const std::string &guid) const;
    std::string pathToGuid(const std::string &path) const;
    bool isPersistent(const std::string &path) const;

    QImage icon(const QFileInfo &source);
    QImage defaultIcon(const QFileInfo &source);

    Actor *createActor(const QString &source);

    QSet<QString> labels() const;

    AssetConverterSettings *fetchSettings(const QFileInfo &source);

    AssetConverter *getConverter(const QFileInfo &source);

    bool isOutdated() const;

    ConverterMap converters() const;
    QList<CodeBuilder *> builders() const;
    ClassMap classMaps() const;

    bool pushToImport(AssetConverterSettings *settings);

public slots:
    void reimport();

    void onBuildSuccessful();

    void checkImportSettings(AssetConverterSettings *settings);

signals:
    void ready();

    void directoryChanged(const QString &path);
    void fileChanged(const QString &path);

    void imported(const QString &path, const QString &type);
    void importStarted(int count, const QString &stage);
    void importFinished();

    void iconUpdated(QString guid);

    void prefabCreated(uint32_t uuid, uint32_t clone);

    void buildSuccessful();

protected slots:
    void onPerform();

    void onFileChanged(const QString &path, bool force = false);

    void onDirectoryChanged(const QString &path, bool force = false);

private:
    AssetManager();
    ~AssetManager();

    static AssetManager *m_instance;

protected:
    ConverterMap m_converters;

    ResourceSystem::DictionaryMap &m_indices;

    VariantMap m_paths;
    QSet<QString> m_labels;

    QFileSystemWatcher *m_dirWatcher;
    QFileSystemWatcher *m_fileWatcher;

    QList<AssetConverterSettings *> m_importQueue;

    ProjectSettings *m_projectManager;

    QTimer *m_timer;

    ClassMap m_classMaps;
    QList<CodeBuilder *> m_builders;

    SettingsMap m_converterSettings;

    QHash<QString, QImage> m_defaultIcons;

    bool m_noIcons;

protected:
    void cleanupBundle();
    void dumpBundle();

    void convert(AssetConverterSettings *settings);

    QString pathToLocal(const QFileInfo &source) const;

    void registerAsset(const QFileInfo &source, const QString &guid, const QString &type);
    std::string unregisterAsset(const std::string &source);

    QImage renderDocumentIcon(QFileInfo path, QString color = QString("#0277bd"));

};

#endif // ASSETMANAGER_H
