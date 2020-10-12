#ifndef BASECONVERTERSETTINGS_H
#define BASECONVERTERSETTINGS_H

#include <QObject>
#include <QMap>

#include <engine.h>

typedef QMap<QString, QString> QStringMap;

class NEXT_LIBRARY_EXPORT IConverterSettings : public QObject {
    Q_OBJECT

public:
    IConverterSettings              ();
    virtual ~IConverterSettings     () {}

    virtual uint32_t        type                    () const;
    virtual void            setType                 (uint32_t type);

    virtual bool            isValid                 () const;
    virtual void            setValid                (bool valid);

    virtual const char     *source                  () const;
    virtual void            setSource               (const char *source);

    virtual const char     *destination             () const;
    virtual void            setDestination          (const char *destination);

    virtual const char     *absoluteDestination     () const;
    virtual void            setAbsoluteDestination  (const char *destination);

    uint32_t                crc                     () const;
    void                    setCRC                  (uint32_t crc);

    uint32_t                version                 () const;
    void                    setVersion              (uint32_t version);

    uint32_t                currentVersion          () const;
    void                    setCurrentVersion       (uint32_t version);

    const QStringList       subKeys                 () const;
    QString                 subItem                 (const QString &key) const;
    int32_t                 subType                 (const QString &key) const;

    void                    setSubItem              (const QString &name, const QString &uuid, int32_t type);

    bool                    loadSettings            ();
    void                    saveSettings            ();

    bool                    isModified              () const { return m_Modified; }
    void                    setModified             () { m_Modified = true; }

signals:
    void                    updated                 ();

protected:
    bool                    m_Valid;
    bool                    m_Modified;

    uint32_t                m_Type;
    uint32_t                m_Version;
    uint32_t                m_CurrentVersion;
    uint32_t                m_CRC;

    string                  m_Destination;
    string                  m_AbsoluteDestination;
    string                  m_Source;

    QStringMap              m_SubItems;
    QMap<QString, int32_t>  m_SubTypes;
};

class NEXT_LIBRARY_EXPORT IConverter : public QObject {
    Q_OBJECT

public:
    enum ContentTypes {
        ContentInvalid = MetaType::USERTYPE,
        ContentText,
        ContentTexture,
        ContentMaterial,
        ContentMesh,
        ContentPrefab,
        ContentFont,
        ContentAnimation,
        ContentEffect,
        ContentSound,
        ContentCode,
        ContentMap,
        ContentPipeline,
        ContentAtlas,
        ContentAnimationStateMachine,
        ContentPhysicMaterial,
        ContentLocalization,
        ContentPose,
        ContentLast
    };
public:
    virtual void init();
    virtual QStringList suffixes() const = 0;
    virtual uint32_t contentType() const = 0;
    virtual uint32_t type() const = 0;
    virtual uint8_t convertFile(IConverterSettings *) = 0;

    virtual IConverterSettings *createSettings() const;

    virtual QString templatePath() const;
};

#endif // BASECONVERTERSETTINGS_H
