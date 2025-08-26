#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include "resources/audioclip.h"

#include <assetconverter.h>

#include <vorbis/vorbisenc.h>

class QAudioDecoder;
class QAudioFormat;
class QEventLoop;

class AudioProxy;

class AudioImportSettings : public AssetConverterSettings {
    A_OBJECT(AudioImportSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTY(bool, Streamed, AudioImportSettings::stream, AudioImportSettings::setStream),
        A_PROPERTY(bool, Force_Mono, AudioImportSettings::mono, AudioImportSettings::setMono),
        A_PROPERTY(float, Quality, AudioImportSettings::quality, AudioImportSettings::setQuality)
    )

public:
    AudioImportSettings();

    bool stream() const;
    void setStream(bool stream);

    bool mono() const;
    void setMono(bool mono);

    float quality() const;
    void setQuality(float quality);

private:
    StringList typeNames() const override;

    TString defaultIconPath(const TString &) const override;

protected:
    float m_quality;

    bool m_stream;

    bool m_mono;

};

class AudioConverter : public AssetConverter {
public:
    AudioConverter();

    StringList suffixes() const override { return {"mp3", "wav", "ogg"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;

    void onBufferReady();
    void onFinished();

protected:
    VariantMap convertResource(AudioImportSettings *, int32_t srcChanels);

    bool readOgg(AssetConverterSettings *settings, int32_t &channels);

private:
    QByteArray m_buffer;

    AudioProxy *m_proxy;

    QAudioDecoder *m_decoder;

    QEventLoop *m_loop;

};

class AudioProxy : public QObject {
    Q_OBJECT
public:
    void setConverter(AudioConverter *converter) {
        m_converter = converter;
    }

public slots:
    void onBufferReady() {
        m_converter->onBufferReady();
    }

    void onFinished() {
        m_converter->onFinished();
    }

private:
    AudioConverter *m_converter;

};

#endif // AUDIOCONVERTER_H
