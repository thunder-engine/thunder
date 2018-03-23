#ifndef BASECONVERTERSETTINGS_H
#define BASECONVERTERSETTINGS_H

#include <QObject>

#include <engine.h>

class IConverterSettings {
public:
    virtual ~IConverterSettings     () {}

    virtual uint8_t                 type                    () const = 0;
    virtual void                    setType                 (uint8_t type) = 0;

    virtual bool                    isValid                 () const = 0;
    virtual void                    setValid                (bool valid) = 0;

    virtual const char             *source                  () const = 0;
    virtual void                    setSource               (const char *source) = 0;

    virtual const char             *destination             () const = 0;
    virtual void                    setDestination          (const char *destination) = 0;

};

class IConverter {
public:
    enum ContentTypes {
        ContentInvalid              = MetaType::USERTYPE,
        ContentText,
        ContentTexture,
        ContentMaterial,
        ContentMesh,
        ContentAnimation,
        ContentEffect,
        ContentSound,
        ContentCode,
        ContentMap,
        ContentLast
    };
public:
    virtual string                  format                  () const = 0;
    virtual ContentTypes            type                    () const = 0;
    virtual uint8_t                 convertFile             (IConverterSettings *) = 0;
};

class BaseConverterSettings : public QObject, public IConverterSettings  {
    Q_OBJECT
public:
    BaseConverterSettings   (QObject *parent = 0);

    uint8_t                 type                    () const;
    void                    setType                 (uint8_t type);

    bool                    isValid                 () const;
    void                    setValid                (bool valid);

    uint32_t                crc                     () const;
    void                    setCRC                  (uint32_t crc);

    const char             *source                  () const;
    void                    setSource               (const char *source);

    const char             *destination             () const;
    void                    setDestination          (const char *destination);

    virtual void            loadProperties          (const QVariantMap &) {}

signals:
    void                    updated                 ();

protected:
    bool                    mValid;

    uint8_t                 mType;

    uint32_t                mCRC;

    string                  mDestination;
    string                  mSource;
};

#endif // BASECONVERTERSETTINGS_H
