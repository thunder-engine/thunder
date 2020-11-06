#ifndef BASECONVERTERSETTINGS_H
#define BASECONVERTERSETTINGS_H

#include <QObject>
#include <QMap>

#include <engine.h>

class Actor;

typedef QMap<QString, QString> QStringMap;

class NEXT_LIBRARY_EXPORT IConverterSettings : public QObject {
    Q_OBJECT

public:
    IConverterSettings();
    virtual ~IConverterSettings() {}

    virtual uint32_t type() const;
    virtual void setType(uint32_t type);

    virtual QString typeName() const;

    virtual bool isValid() const;
    virtual void setValid(bool valid);

    virtual QString source() const;
    virtual void setSource(const QString &source);

    virtual QString destination() const;
    virtual void setDestination(const QString &destination);

    virtual QString absoluteDestination() const;
    virtual void setAbsoluteDestination(const QString &destination);

    uint32_t crc() const;
    void setCRC(uint32_t crc);

    uint32_t version() const;
    void setVersion(uint32_t version);

    uint32_t currentVersion() const;
    void setCurrentVersion(uint32_t version);

    const QStringList subKeys() const;
    QString subItem(const QString &key) const;
    QString subTypeName(const QString &key) const;
    int32_t subType(const QString &key) const;

    void setSubItem(const QString &name, const QString &uuid, int32_t type);

    bool loadSettings();
    void saveSettings();

    bool isModified() const { return m_Modified; }
    void setModified() { m_Modified = true; }

signals:
    void updated();

protected:
    bool m_Valid;
    bool m_Modified;

    uint32_t m_Type;
    uint32_t m_Version;
    uint32_t m_CurrentVersion;
    uint32_t m_CRC;

    QString m_Destination;
    QString m_AbsoluteDestination;
    QString m_Source;

    QStringMap m_SubItems;
    QStringMap m_SubTypeNames;
    QMap<QString, int32_t> m_SubTypes;
};

typedef QList<uint32_t> QIntegerList;

class NEXT_LIBRARY_EXPORT IConverter : public QObject {
    Q_OBJECT

public:
    virtual void init();
    virtual QStringList suffixes() const = 0;

    virtual uint8_t convertFile(IConverterSettings *settings) = 0;
    virtual IConverterSettings *createSettings() const = 0;

    virtual QString templatePath() const;
    virtual QString iconPath() const;

    virtual Actor *createActor(const QString &guid) const;
};

#endif // BASECONVERTERSETTINGS_H
