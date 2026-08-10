#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <QObject>
#include <QTimer>

#include <set>

#include <editor.h>

#include <systems/resourcesystem.h>

class BaseAssetProvider;
class AssetConverter;
class AssetConverterSettings;
class CodeBuilder;

class EDITOR_EXPORT AssetManager : public QObject {
    Q_OBJECT

public:
    AssetManager();
    ~AssetManager();

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
    AssetConverter *getConverter(const TString &source);

    std::list<AssetConverter *> converters() const;
    std::list<CodeBuilder *> builders() const;

    static void findFreeName(TString &name, const TString &path, const TString &suff = TString());

    TString uuidToPath(const TString &uuid) const;
    TString pathToUuid(const TString &path) const;
    TString pathToLocal(const TString &source) const;

    Actor *createActor(const TString &source);

    StringList labels() const;

    AssetConverterSettings *fetchSettings(const TString &source);

    bool pushToImport(AssetConverterSettings *settings);

    void createFromTemplate(const TString &destination);

    void registerAsset(const TString &source, const ResourceSystem::ResourceInfo &info);
    TString unregisterAsset(const TString &source);

    void dumpBundle();

public slots:
    void reimport();

    void onBuildSuccessful(bool flag, CodeBuilder *builder);

signals:
    void directoryChanged(const TString &path);

    void imported();
    void importStarted(int count, const TString &stage);
    void importFinished();

    void iconUpdated(const TString &guid);

    void prefabCreated(uint32_t uuid, uint32_t clone);

    void buildSuccessful(bool flag);

protected slots:
    void onPerform();

protected:
    friend class BaseAssetProvider;

    std::map<TString, AssetConverter *> m_converters;

    std::map<TString, AssetConverterSettings *> m_converterSettings;

    std::map<TString, TString> m_paths;

    std::set<TString> m_labels;

    std::list<CodeBuilder *> m_builders;

    std::list<AssetConverterSettings *> m_importQueue;

    std::list<std::pair<TString, TString>> m_changedUUIDs;

    BaseAssetProvider *m_assetProvider;

    QTimer *m_timer;

    bool m_force;

protected:
    void convert(AssetConverterSettings *settings);

    void getChangedUUIDs();

    void fixUUIDs();
};

#endif // ASSETMANAGER_H
