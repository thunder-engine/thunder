#include "converters/audioconverter.h"

#include <QAudioDecoder>
#include <QEventLoop>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QUrl>

#include <ctime>

#include <bson.h>
#include <log.h>

#include <vorbis/vorbisfile.h>

#define HEADER      "Header"
#define BLOCK_SIZE  1024
#define FORMAT_VERSION 1

AudioImportSettings::AudioImportSettings() :
        AssetConverterSettings(),
        m_quality(1.0f),
        m_stream(false),
        m_mono(false) {

    setType(MetaType::type<AudioClip *>());
    setVersion(FORMAT_VERSION);
}

bool AudioImportSettings::stream() const {
    return m_stream;
}

QString AudioImportSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/audio.svg";
}

void AudioImportSettings::setStream(bool stream) {
    m_stream = stream;
}

bool AudioImportSettings::mono() const {
    return m_mono;
}

void AudioImportSettings::setMono(bool mono) {
    m_mono = mono;
}

float AudioImportSettings::quality() const {
    return m_quality;
}

void AudioImportSettings::setQuality(float quality) {
    m_quality = quality;
}

AudioConverter::AudioConverter() :
        m_proxy(new AudioProxy),
        m_decoder(new QAudioDecoder(m_proxy)),
        m_loop(new QEventLoop(m_proxy)) {

    m_proxy->setConverter(this);

    QObject::connect(m_decoder, &QAudioDecoder::bufferReady, m_proxy, &AudioProxy::onBufferReady);
    QObject::connect(m_decoder, &QAudioDecoder::finished, m_proxy, &AudioProxy::onFinished);

    AudioProxy::connect(m_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), [=](QAudioDecoder::Error error) {

        m_loop->exit(error);
    });
}

AssetConverter::ReturnCode AudioConverter::convertFile(AssetConverterSettings *settings) {
    m_buffer.clear();

    int32_t channels = 1;
    QFileInfo info(settings->source());

    if(info.suffix() == "ogg") {
        readOgg(settings, channels);
    } else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_decoder->setSourceFilename(settings->source());
#else
        m_decoder->setSource(QUrl(settings->source()));
#endif
        m_decoder->start();

        int32_t code = m_loop->exec();
        if(code != QAudioDecoder::NoError) {
            aError() << "Unable to convert:" << info.fileName().toStdString() << "error code:" << code;

            return InternalError;
        }

        channels = m_decoder->audioFormat().channelCount();
    }

    AudioClip clip;
    VariantMap data = convertResource(static_cast<AudioImportSettings *>(settings), channels);
    clip.loadUserData(data);

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save( Engine::toVariant(&clip) );
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
    }

    return Success;
}

VariantMap AudioConverter::convertResource(AudioImportSettings *settings, int32_t srcChanels) {
    VariantMap result;

    ogg_stream_state stream;
    ogg_page page;

    vorbis_dsp_state state;
    vorbis_block block;
    vorbis_info info;
    vorbis_comment comment;

    vorbis_info_init(&info);
    vorbis_comment_init(&comment);

    int32_t channels = srcChanels;
    if(settings->mono()) {
        channels = 1;
    }
    vorbis_encode_init_vbr(&info, channels, 44100, CLAMP(settings->quality(), 0.0, 1.0));
    vorbis_comment_add_tag(&comment, "encoder", "thunder");

    vorbis_analysis_init(&state, &info);
    vorbis_block_init(&state, &block);

    srand(time(nullptr));
    ogg_stream_init(&stream, rand());

    ogg_packet header_packet;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&state, &comment, &header_packet, &header_comm, &header_code);
    ogg_stream_packetin(&stream, &header_packet);
    ogg_stream_packetin(&stream, &header_comm);
    ogg_stream_packetin(&stream, &header_code);

    QString path("stream");
    QString uuid = settings->subItem(path);
    if(uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString();
        settings->setSubItem(path, uuid, 0);
    }
    QFileInfo dst(settings->absoluteDestination());

    AudioClip *clip = Engine::loadResource<AudioClip>(settings->destination().toStdString());
    if(clip) {
        clip->unloadAudioData();
    }

    QFile file(dst.absolutePath() + "/" + uuid);
    if(file.open(QIODevice::WriteOnly)) {
        while(true) {
            if(ogg_stream_flush(&stream, &page) == 0) {
                break;
            }
            file.write(reinterpret_cast<const char *>(page.header), page.header_len);
            file.write(reinterpret_cast<const char *>(page.body), page.body_len);
        }

        QBuffer buffer(&m_buffer);
        buffer.open(QIODevice::ReadOnly);

        int64_t offset = 2 * srcChanels;

        char *ptr = new char[BLOCK_SIZE * offset];

        bool eof = false;
        while(!eof) {
            int64_t bytes = buffer.read(ptr, BLOCK_SIZE * offset);
            if(bytes == 0) {
                vorbis_analysis_wrote(&state, 0);
            } else {
                float **data = vorbis_analysis_buffer(&state, bytes / offset);
                uint64_t i;
                for(i = 0; i < bytes / offset; i++) {
                    data[0][i]  = ((ptr[i*offset+1] << 8) | (0x00ff & (int)ptr[i * offset])) / 32768.f;
                    if(channels > 1) {
                        data[1][i]  = ((ptr[i*offset+3] << 8) | (0x00ff & (int)ptr[i*offset+2])) / 32768.f;
                    }
                }
                vorbis_analysis_wrote(&state, i);
            }

            while(vorbis_analysis_blockout(&state, &block) == 1) {
                vorbis_analysis(&block, nullptr);
                vorbis_bitrate_addblock(&block);

                ogg_packet packet;
                while(vorbis_bitrate_flushpacket(&state, &packet)) {
                    ogg_stream_packetin(&stream, &packet);

                    while(!eof) {
                        if(ogg_stream_pageout(&stream, &page) == 0) {
                            break;
                        }
                        file.write(reinterpret_cast<const char *>(page.header), page.header_len);
                        file.write(reinterpret_cast<const char *>(page.body), page.body_len);

                        if(ogg_page_eos(&page)) {
                            eof = true;
                        }
                    }
                }
            }
        }
        file.close();

        delete []ptr;
    }

    if(clip) {
        clip->loadAudioData();
    }

    ogg_stream_clear(&stream);
    vorbis_block_clear(&block);
    vorbis_dsp_clear(&state);
    vorbis_comment_clear(&comment);
    vorbis_info_clear(&info);

    VariantList header;
    header.push_back(TString(uuid.toStdString()));
    header.push_back(settings->stream());
    result[HEADER]  = header;

    return result;
}

AssetConverterSettings *AudioConverter::createSettings() {
    return new AudioImportSettings();
}

void AudioConverter::onBufferReady() {
    QAudioBuffer buffer = m_decoder->read();

    m_buffer.append(buffer.constData<char>(), buffer.byteCount());
}

void AudioConverter::onFinished() {
    m_loop->exit();
}

bool AudioConverter::readOgg(AssetConverterSettings *settings, int32_t &channels) {
    OggVorbis_File vorbisFile;
    if(ov_fopen(qPrintable(settings->source()), &vorbisFile) < 0) {
        return false;
    }

    vorbis_info *info = ov_info(&vorbisFile, -1);
    channels = info->channels;

    char out[BLOCK_SIZE];
    while(true) {
        int32_t	section = 0;
        int32_t result = 0;
        while(result < BLOCK_SIZE) {
            int32_t length  = ov_read(&vorbisFile, out + result, BLOCK_SIZE - result, 0, 2, 1, &section);
            if(length <= 0) {
                return true;
            }
            result += length;
        }
        m_buffer.append(out, result);
    }
}
