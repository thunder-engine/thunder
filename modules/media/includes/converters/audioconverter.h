#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QAudioDecoder>

#include "resources/audioclip.h"

#include "converter.h"

#include <vorbis/vorbisenc.h>

class QAudioDecoder;
class QAudioFormat;
class QEventLoop;

class AudioImportSettings : public QObject, public IConverterSettings {
    Q_OBJECT

    Q_PROPERTY(bool Streamed READ stream WRITE setStream DESIGNABLE true USER true)
    Q_PROPERTY(bool Force_Mono READ mono WRITE setMono DESIGNABLE true USER true)
    Q_PROPERTY(float Quality READ quality WRITE setQuality DESIGNABLE true USER true)

public:
    AudioImportSettings         (QObject *parent = 0);

    bool                        stream                      () const;
    void                        setStream                   (bool stream);

    bool                        mono                        () const;
    void                        setMono                     (bool mono);

    float                       quality                     () const;
    void                        setQuality                  (float quality);

    void                        loadProperties              (const QVariantMap &map);

protected:
    bool                        m_Stream;

    bool                        m_Mono;

    float                       m_Quality;
};

class AudioClipSerial : public AudioClip {
public:
    VariantMap                  saveUserData                () const;

protected:
    friend class AudioConverter;

};

class AudioConverter : public IConverter {
    Q_OBJECT
public:
    AudioConverter              ();

    QStringList                 suffixes                    () const { return {"mp3", "wav", "ogg"}; }
    uint32_t                    contentType                 () const { return IConverter::ContentSound; }
    uint32_t                    type                        () const { return MetaType::type<AudioClip *>(); }
    uint8_t                     convertFile                 (IConverterSettings *);

    VariantMap                  convertResource             (AudioImportSettings *, uint32_t srcChanels);

    IConverterSettings         *createSettings              () const;

public slots:
    void                        onBufferReady               ();
    void                        onFinished                  ();

protected:
    bool                        readOgg                     (IConverterSettings *settings, uint32_t &channels);

    QAudioDecoder              *m_pDecoder;

    QEventLoop                 *m_pLoop;

    QByteArray                  m_Buffer;
};

#endif // AUDIOCONVERTER_H
