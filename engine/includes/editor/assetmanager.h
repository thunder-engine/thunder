#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <QObject>
#include <QTimer>
#include <QImage>

#include <engine.h>
#include <module.h>
#include <url.h>

#include <editor/assetconverter.h>

#include <systems/resourcesystem.h>

class ProjectSettings;

class CodeBuilder;

class BaseAssetProvider;

class ENGINE_EXPORT AssetManager : public QObject {
    Q_OBJECT

public:
    typedef std::map<TString, AssetConverter *> ConverterMap;
    typedef std::map<TString, AssetConverterSettings *> SettingsMap;

public:
    static AssetManager *instance();
    static void destroy();

    void init();

    void rescan();

    TString assetTypeName(const TString &source);

    void removeResource(const TString &source);
    void renameResource(const TString &oldName, const TString &newName);
    void duplicateResource(const TString &source);

    void makePrefab(const TString &source, const TString &target);

    bool pushToImport(const TString &source);
    bool import(const TString &source, const TString &target);

    void registerConverter(AssetConverter *converter);

    static void findFreeName(TString &name, const TString &path, const TString &suff = TString());

    TString guidToPath(const TString &guid) const;
    TString pathToGuid(const TString &path) const;
    bool isPersistent(const TString &path) const;

    QImage icon(const TString &source);

    Actor *createActor(const TString &source);

    std::set<TString> labels() const;

    AssetConverterSettings *fetchSettings(const TString &source);

    AssetConverter *getConverter(const TString &source);

    StringList templates() const;

    std::list<CodeBuilder *> builders() const;

    bool pushToImport(AssetConverterSettings *settings);

public slots:
    void reimport();

    void onBuildSuccessful(CodeBuilder *builder);

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

    std::list<CodeBuilder *> m_builders;

    SettingsMap m_converterSettings;

    VariantMap m_paths;
    std::set<TString> m_labels;

    std::list<AssetConverterSettings *> m_importQueue;

    BaseAssetProvider *m_assetProvider;

    ResourceSystem::DictionaryMap &m_indices;

    ProjectSettings *m_projectManager;

    QTimer *m_timer;

protected:
    void dumpBundle();

    void convert(AssetConverterSettings *settings);

    TString pathToLocal(const TString &source) const;

    void registerAsset(const TString &source, const TString &guid, const TString &type);
    TString unregisterAsset(const TString &source);

};

#endif // ASSETMANAGER_H
