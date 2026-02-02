#ifndef ASSETCONVERTER_H
#define ASSETCONVERTER_H

#include <QImage>

#include <engine.h>
#include <systems/resourcesystem.h>

class Actor;
class AssetConverterSettings;

class ENGINE_EXPORT AssetConverter : public Object {
public:
    enum ReturnCode {
        Success = 0,
        InternalError,
        Unsupported,
        Skipped
    };

    virtual void init();
    virtual StringList suffixes() const = 0;

    virtual ReturnCode convertFile(AssetConverterSettings *settings) = 0;
    virtual AssetConverterSettings *createSettings() = 0;

    virtual void renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName);

    virtual void createFromTemplate(const TString &destination);

    virtual TString templatePath() const;

    virtual Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const;

};

class ENGINE_EXPORT AssetConverterSettings : public Object {
    A_OBJECT(AssetConverterSettings, Object, Editor)

    A_METHODS(
        A_SIGNAL(AssetConverterSettings::updated)
    )

    struct SubItem {
        ResourceSystem::ResourceInfo info;

        QImage icon;

        bool dirty = true;
    };

public:
    AssetConverterSettings();
    ~AssetConverterSettings();

    AssetConverter *converter();
    void setConverter(AssetConverter *converter);

    uint32_t type() const;

    virtual StringList typeNames() const;
    TString typeName() const override;

    virtual bool isReadOnly() const;

    virtual bool isOutdated() const;

    virtual bool isCode() const;

    virtual bool isDir() const;

    TString source() const;
    void setSource(const TString &source);

    TString destination() const;

    TString absoluteDestination() const;

    void resetIcon(const TString &uuid);
    QImage icon(const TString &uuid);

    TString hash() const;

    ResourceSystem::ResourceInfo &info() const;

    uint32_t version() const;

    uint32_t currentVersion() const;
    void setCurrentVersion(uint32_t version);

    const StringList subKeys() const;

    void setSubItemsDirty();

    ResourceSystem::ResourceInfo subItem(const TString &key, bool create = false) const;
    void setSubItem(const TString &name, const ResourceSystem::ResourceInfo &info);

    Variant subItemData(const TString &key) const;
    virtual void setSubItemData(const TString &name, const Variant &data);

    AssetConverter::ReturnCode saveBinary(const Variant &data, const TString &path);

    bool loadSettings();
    void saveSettings();

    bool isModified() const;
    void setModified();

    void setDirectory();

    static QImage documentIcon(const TString &type);
    static void setDefaultIconPath(const TString &type, const TString &path);

signals:
    void updated();

protected:
    virtual TString propertyAllias(const TString &name) const;

    static TString defaultIconPath(const TString &type);

    void setVersion(uint32_t version);

    static QImage renderDocumentIcon(const TString &path, const TString &color = TString("#0277bd"));

protected:
    bool m_valid;
    bool m_modified;
    bool m_dir;

    uint32_t m_version;
    uint32_t m_currentVersion;

    mutable ResourceSystem::ResourceInfo m_info;

    TString m_source;
    TString m_suffix;

    QImage m_icon;

    std::map<TString, SubItem> m_subItems;

    static std::map<TString, TString> m_defaultIcons;

    AssetConverter *m_converter;

};

#endif // ASSETCONVERTER_H
