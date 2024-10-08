#ifndef ASSETCONVERTER_H
#define ASSETCONVERTER_H

#include <QObject>
#include <QMap>
#include <QImage>

#include <engine.h>

class Actor;

typedef QMap<QString, QString> QStringMap;

class ENGINE_EXPORT AssetConverterSettings : public QObject {
    Q_OBJECT

public:
    AssetConverterSettings();
    virtual ~AssetConverterSettings();

    virtual uint32_t type() const;
    virtual void setType(uint32_t type);

    virtual QStringList typeNames() const;
    virtual QString typeName() const;

    virtual bool isValid() const;
    virtual void setValid(bool valid);

    virtual bool isReadOnly() const;

    virtual bool isOutdated() const;

    virtual bool isCode() const;

    virtual QString source() const;
    virtual void setSource(const QString &source);

    virtual QString destination() const;
    virtual void setDestination(const QString &destination);

    virtual QString absoluteDestination() const;
    virtual void setAbsoluteDestination(const QString &destination);

    virtual QString defaultIcon(QString type) const;

    QString hash() const;
    void setHash(const QString &hash);

    uint32_t version() const;
    void setVersion(uint32_t version);

    uint32_t currentVersion() const;
    void setCurrentVersion(uint32_t version);

    const QStringList subKeys() const;
    QString subItem(const QString &key) const;
    virtual QJsonObject subItemData(const QString &key) const;
    QString subTypeName(const QString &key) const;
    int32_t subType(const QString &key) const;

    void setSubItem(const QString &name, const QString &uuid, int32_t type);
    virtual void setSubItemData(const QString &name, const QJsonObject &data);

    QString saveSubData(const ByteArray &data, const QString &path, int32_t type);

    bool loadSettings();
    void saveSettings();

    bool isModified() const;
    void setModified();

signals:
    void updated();

protected:
    bool m_valid;
    bool m_modified;

    uint32_t m_type;
    uint32_t m_version;
    uint32_t m_currentVersion;

    mutable QString m_md5;
    QString m_destination;
    QString m_absoluteDestination;
    QString m_source;

    QStringMap m_subItems;
    QStringMap m_subTypeNames;
    QMap<QString, int32_t> m_subTypes;
};

typedef QList<uint32_t> QIntegerList;

class ENGINE_EXPORT AssetConverter : public QObject {
    Q_OBJECT

public:
    enum ReturnCode {
        Success = 0,
        InternalError,
        Unsupported,
        Skipped,
        CopyAsIs
    };

    virtual void init();
    virtual QStringList suffixes() const = 0;

    virtual ReturnCode convertFile(AssetConverterSettings *settings) = 0;
    virtual AssetConverterSettings *createSettings() = 0;

    virtual void renameAsset(AssetConverterSettings *settings, const QString &oldName, const QString &newName);

    virtual QString templatePath() const;
    virtual QString iconPath() const;

    virtual Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const;
};

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

#endif // ASSETCONVERTER_H
