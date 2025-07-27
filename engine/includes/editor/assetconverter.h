#ifndef ASSETCONVERTER_H
#define ASSETCONVERTER_H

#include <QObject>
#include <QMap>
#include <QImage>

#include <engine.h>

class Actor;

class ENGINE_EXPORT AssetConverterSettings : public QObject {
    Q_OBJECT

    struct SubItem {
        QString uuid;

        QImage icon;

        int32_t typeId;

        bool dirty = true;
    };

public:
    AssetConverterSettings();
    virtual ~AssetConverterSettings();

    uint32_t type() const;

    virtual QStringList typeNames() const;
    virtual QString typeName() const;

    virtual bool isReadOnly() const;

    virtual bool isOutdated() const;

    virtual bool isCode() const;

    virtual bool isDir() const;

    virtual QString source() const;
    virtual void setSource(const QString &source);

    virtual QString destination() const;
    virtual void setDestination(const QString &destination);

    virtual QString absoluteDestination() const;
    virtual void setAbsoluteDestination(const QString &destination);

    void resetIcon(const QString &uuid);
    QImage icon(const QString &uuid);

    QString hash() const;
    void setHash(const QString &hash);

    uint32_t version() const;

    uint32_t currentVersion() const;
    void setCurrentVersion(uint32_t version);

    const QStringList subKeys() const;
    QString subItem(const QString &key) const;
    virtual QJsonObject subItemData(const QString &key) const;
    QString subTypeName(const QString &key) const;
    int32_t subType(const QString &key) const;

    void setSubItemsDirty();

    void setSubItem(const QString &name, const QString &uuid, int32_t type);
    virtual void setSubItemData(const QString &name, const QJsonObject &data);

    QString saveSubData(const ByteArray &data, const QString &path, int32_t type);

    bool loadSettings();
    void saveSettings();

    bool isModified() const;
    void setModified();

    void setDirectory();

    QImage documentIcon(const QString &type);

signals:
    void updated();

protected:
    virtual QString defaultIconPath(const QString &type) const;

    void setType(uint32_t type);

    void setVersion(uint32_t version);

    QImage renderDocumentIcon(const QString &path, const QString &color = QString("#0277bd"));

protected:
    bool m_valid;
    bool m_modified;
    bool m_dir;

    uint32_t m_type;
    uint32_t m_version;
    uint32_t m_currentVersion;

    mutable QString m_md5;
    QString m_destination;
    QString m_absoluteDestination;
    QString m_source;

    QImage m_icon;

    QMap<QString, SubItem> m_subItems;
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
