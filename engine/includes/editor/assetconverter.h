#ifndef ASSETCONVERTER_H
#define ASSETCONVERTER_H

#include <QObject>
#include <QMap>

#include <engine.h>

class Actor;

typedef QMap<QString, QString> QStringMap;

class NEXT_LIBRARY_EXPORT AssetConverterSettings : public QObject {
    Q_OBJECT

public:
    AssetConverterSettings();
    virtual ~AssetConverterSettings();

    virtual uint32_t type() const;
    virtual void setType(uint32_t type);

    virtual QString typeName() const;

    virtual bool isValid() const;
    virtual void setValid(bool valid);

    virtual bool isReadOnly() const;

    virtual QString source() const;
    virtual void setSource(const QString &source);

    virtual QString destination() const;
    virtual void setDestination(const QString &destination);

    virtual QString absoluteDestination() const;
    virtual void setAbsoluteDestination(const QString &destination);

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

    bool loadSettings();
    void saveSettings();

    bool isModified() const;
    void setModified();

signals:
    void updated();

protected:
    bool m_Valid;
    bool m_Modified;

    uint32_t m_Type;
    uint32_t m_Version;
    uint32_t m_CurrentVersion;

    QString m_Md5;
    QString m_Destination;
    QString m_AbsoluteDestination;
    QString m_Source;

    QStringMap m_SubItems;
    QStringMap m_SubTypeNames;
    QMap<QString, int32_t> m_SubTypes;
};

typedef QList<uint32_t> QIntegerList;

class NEXT_LIBRARY_EXPORT AssetConverter : public QObject {
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
    virtual AssetConverterSettings *createSettings() const = 0;

    virtual QString templatePath() const;
    virtual QString iconPath() const;

    virtual Actor *createActor(const QString &guid) const;
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
