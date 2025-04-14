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

class ProjectSettings;

class CodeBuilder;

class BaseAssetProvider;

class ENGINE_EXPORT AssetManager : public QObject {
    Q_OBJECT

public:
    typedef QMap<QString, AssetConverter *> ConverterMap;
    typedef QMap<QString, AssetConverterSettings *> SettingsMap;

public:
    static AssetManager *instance();
    static void destroy();

    void init();

    void rescan(bool force);

    void setNoIcons();

    QString assetTypeName(const QFileInfo &source);

    void removeResource(const QString &source);
    void renameResource(const QString &oldName, const QString &newName);
    void duplicateResource(const QString &source);

    void makePrefab(const QString &source, const QString &target);

    bool pushToImport(const QString &source);
    bool import(const QString &source, const QString &target);

    void registerConverter(AssetConverter *converter);

    static void findFreeName(QString &name, const QString &path, const QString &suff = QString());

    std::string guidToPath(const std::string &guid) const;
    std::string pathToGuid(const std::string &path) const;
    bool isPersistent(const std::string &path) const;

    QImage icon(const QString &source);
    QImage defaultIcon(const QString &source);

    Actor *createActor(const QString &source);

    QSet<QString> labels() const;

    AssetConverterSettings *fetchSettings(const QString &source);

    AssetConverter *getConverter(const QString &source);

    QStringList templates() const;

    QList<CodeBuilder *> builders() const;

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

private:
    AssetManager();
    ~AssetManager();

    static AssetManager *m_instance;

protected:
    friend class BaseAssetProvider;

    ConverterMap m_converters;

    QList<CodeBuilder *> m_builders;

    SettingsMap m_converterSettings;

    VariantMap m_paths;
    QSet<QString> m_labels;

    QList<AssetConverterSettings *> m_importQueue;

    QHash<QString, QImage> m_defaultIcons;

    BaseAssetProvider *m_assetProvider;

    ResourceSystem::DictionaryMap &m_indices;

    ProjectSettings *m_projectManager;

    QTimer *m_timer;

    bool m_noIcons;

protected:
    void dumpBundle();

    void convert(AssetConverterSettings *settings);

    std::string pathToLocal(const QString &source) const;

    void registerAsset(const QString &source, const QString &guid, const QString &type);
    QString unregisterAsset(const QString &source);

    QImage renderDocumentIcon(const QString &path, const QString &color = QString("#0277bd"));

};

#endif // ASSETMANAGER_H
