#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QAudioDecoder>

#include "resources/audioclip.h"

#include <assetconverter.h>

#include <vorbis/vorbisenc.h>

class QAudioDecoder;
class QAudioFormat;
class QEventLoop;

class AudioImportSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(bool Streamed READ stream WRITE setStream DESIGNABLE true USER true)
    Q_PROPERTY(bool Force_Mono READ mono WRITE setMono DESIGNABLE true USER true)
    Q_PROPERTY(float Quality READ quality WRITE setQuality DESIGNABLE true USER true)

public:
    AudioImportSettings();

    bool stream() const;
    void setStream(bool stream);

    bool mono() const;
    void setMono(bool mono);

    float quality() const;
    void setQuality(float quality);

private:
    QString defaultIconPath(const QString &) const override;

protected:
    float m_quality;

    bool m_stream;

    bool m_mono;

};

class AudioConverter : public AssetConverter {
    Q_OBJECT
public:
    AudioConverter();

    QStringList suffixes() const override { return {"mp3", "wav", "ogg"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;

public slots:
    void onBufferReady();
    void onFinished();

protected:
    VariantMap convertResource(AudioImportSettings *, int32_t srcChanels);

    bool readOgg(AssetConverterSettings *settings, int32_t &channels);

private:
    QByteArray m_buffer;

    QAudioDecoder *m_decoder;

    QEventLoop *m_loop;

};

#endif // AUDIOCONVERTER_H
