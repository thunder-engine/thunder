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

    const QStringList       subKeys                 () const;
    QString                 subItem                 (const QString &key) const;
    int32_t                 subType                 (const QString &key) const;

    void                    setSubItem              (const QString &name, const QString &uuid, int32_t type);

signals:
    void                    updated                 ();

protected:
    bool                    mValid;

    uint32_t                mType;

    uint32_t                mCRC;

    string                  mDestination;
    string                  mAbsoluteDestination;
    string                  mSource;

    QStringMap              mSubItems;
    QMap<QString, int32_t>  mSubTypes;
};

class NEXT_LIBRARY_EXPORT IConverter : public QObject {
    Q_OBJECT

public:
    enum ContentTypes {
        ContentInvalid              = MetaType::USERTYPE,
        ContentText,
        ContentTexture,
        ContentMaterial,
        ContentMesh,
        ContentAtlas,
        ContentFont,
        ContentAnimation,
        ContentEffect,
        ContentSound,
        ContentCode,
        ContentMap,
        ContentPipeline,
        ContentPrefab,
        ContentAnimationStateMachine,
        ContentPhysicMaterial,
        ContentLast
    };
public:
    virtual QStringList             suffixes        () const = 0;
    virtual uint32_t                contentType     () const = 0;
    virtual uint32_t                type            () const = 0;
    virtual uint8_t                 convertFile     (IConverterSettings *) = 0;

    virtual IConverterSettings     *createSettings  () const;
};

#endif // BASECONVERTERSETTINGS_H
