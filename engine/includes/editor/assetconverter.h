#ifndef ASSETCONVERTER_H
#define ASSETCONVERTER_H

#include <QImage>
#include <QObject>

#include <engine.h>

class Actor;

class ENGINE_EXPORT AssetConverterSettings : public Object {
    A_OBJECT(AssetConverterSettings, Object, Editor)

    A_METHODS(
        A_SIGNAL(AssetConverterSettings::updated)
    )

    struct SubItem {
        TString uuid;

        QImage icon;

        int32_t typeId;

        bool dirty = true;
    };

public:
    AssetConverterSettings();
    ~AssetConverterSettings();

    uint32_t type() const;

    virtual StringList typeNames() const;
    TString typeName() const override;

    virtual bool isReadOnly() const;

    virtual bool isOutdated() const;

    virtual bool isCode() const;

    virtual bool isDir() const;

    virtual TString source() const;
    virtual void setSource(const TString &source);

    virtual TString destination() const;
    virtual void setDestination(const TString &destination);

    virtual TString absoluteDestination() const;
    virtual void setAbsoluteDestination(const TString &destination);

    void resetIcon(const TString &uuid);
    QImage icon(const TString &uuid);

    TString hash() const;
    void setHash(const TString &hash);

    uint32_t version() const;

    uint32_t currentVersion() const;
    void setCurrentVersion(uint32_t version);

    const StringList subKeys() const;
    TString subItem(const TString &key) const;
    virtual Variant subItemData(const TString &key) const;
    TString subTypeName(const TString &key) const;
    int32_t subType(const TString &key) const;

    void setSubItemsDirty();

    void setSubItem(const TString &name, const TString &uuid, int32_t type);
    virtual void setSubItemData(const TString &name, const Variant &data);

    TString saveSubData(const ByteArray &data, const TString &path, int32_t type);

    bool loadSettings();
    void saveSettings();

    bool isModified() const;
    void setModified();

    void setDirectory();

    QImage documentIcon(const TString &type);

signals:
    void updated();

protected:
    virtual TString defaultIconPath(const TString &type) const;

    void setType(uint32_t type);

    void setVersion(uint32_t version);

    QImage renderDocumentIcon(const TString &path, const TString &color = TString("#0277bd"));

protected:
    bool m_valid;
    bool m_modified;
    bool m_dir;

    uint32_t m_type;
    uint32_t m_version;
    uint32_t m_currentVersion;

    mutable TString m_md5;
    TString m_destination;
    TString m_absoluteDestination;
    TString m_source;

    QImage m_icon;

    std::map<TString, SubItem> m_subItems;

};

class ENGINE_EXPORT AssetConverter : public Object {
public:
    enum ReturnCode {
        Success = 0,
        InternalError,
        Unsupported,
        Skipped,
        CopyAsIs
    };

    virtual void init();
    virtual StringList suffixes() const = 0;

    virtual ReturnCode convertFile(AssetConverterSettings *settings) = 0;
    virtual AssetConverterSettings *createSettings() = 0;

    virtual void renameAsset(AssetConverterSettings *settings, const TString &oldName, const TString &newName);

    virtual void createFromTemplate(const TString &destination);

    virtual TString templatePath() const;
    virtual TString iconPath() const;

    virtual Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const;
};

struct Template {
    Template() :
            type(MetaType::name(MetaType::INVALID)) {

    }
    Template(const QString &p, const QString &t) :
            path(p),
            type(t) {

        type = type.replace("*", "");
        type = type.trimmed();
    }

    QString path;
    QString type;
};
Q_DECLARE_METATYPE(Template)

#endif // ASSETCONVERTER_H
