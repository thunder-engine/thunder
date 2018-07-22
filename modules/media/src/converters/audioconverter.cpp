#include "converters/audioconverter.h"

#include <QEventLoop>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

#include <ctime>

#include <bson.h>
#include <vorbis/vorbisfile.h>

#define HEADER      "Header"
#define BLOCK_SIZE  1024

AudioImportSettings::AudioImportSettings(QObject *parent) :
        IConverterSettings(),
        m_Stream(false),
        m_Mono(false),
        m_Quality(1.0) {

}

bool AudioImportSettings::stream() const {
    return m_Stream;
}

void AudioImportSettings::setStream(bool stream) {
    m_Stream    = stream;
}

bool AudioImportSettings::mono() const {
    return m_Mono;
}

void AudioImportSettings::setMono(bool mono) {
    m_Mono  = mono;
}

float AudioImportSettings::quality() const {
    return m_Quality;
}

void AudioImportSettings::setQuality(float quality) {
    m_Quality   = quality;
}

void AudioImportSettings::loadProperties(const QVariantMap &map) {
    auto it  = map.find("Streamed");
    if(it != map.end()) {
        setStream( it.value().toBool() );
    }

    it  = map.find("Force_Mono");
    if(it != map.end()) {
        setMono( it.value().toBool() );
    }

    it  = map.find("Quality");
    if(it != map.end()) {
        setQuality( it.value().toFloat() );
    }
}

VariantMap AudioClipSerial::saveUserData() const {
    VariantMap result;

    VariantList header;
    header.push_back(m_Path);
    header.push_back(m_Stream);
    result[HEADER]  = header;

    return result;
}

AudioConverter::AudioConverter() :
        m_pDecoder(new QAudioDecoder(this)),
        m_pLoop(new QEventLoop(this)) {

    connect(m_pDecoder, &QAudioDecoder::bufferReady, this, &AudioConverter::onBufferReady);
    connect(m_pDecoder, &QAudioDecoder::finished, this, &AudioConverter::onFinished);
}

uint8_t AudioConverter::convertFile(IConverterSettings *settings) {
    m_Buffer.clear();

    uint32_t channels   = 1;
    QFileInfo info(settings->source());

    if(info.suffix() == "ogg") {
        readOgg(settings, channels);
    } else {
        m_pDecoder->setSourceFilename(settings->source());
        m_pDecoder->start();

        m_pLoop->exec();

        channels    = m_pDecoder->audioFormat().channelCount();
    }

    AudioClipSerial clip;
    VariantMap data    = convertResource(static_cast<AudioImportSettings *>(settings), channels);
    clip.loadUserData(data);

    QFile file(settings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data  = Bson::save( Engine::toVariant(&clip) );
        file.write((const char *)&data[0], data.size());
        file.close();
    }

    return 0;
}

VariantMap AudioConverter::convertResource(AudioImportSettings *settings, uint32_t srcChanels) {
    VariantMap result;

    ogg_stream_state    stream;
    ogg_page            page;

    vorbis_dsp_state    state;
    vorbis_block        block;
    vorbis_info         info;
    vorbis_comment      comment;

    vorbis_info_init(&info);
    vorbis_comment_init(&comment);

    uint32_t channels   = srcChanels;
    if(settings->mono()) {
        channels    = 1;
    }
    vorbis_encode_init_vbr(&info, channels, 44100, CLAMP(settings->quality(), 0.0, 1.0));
    vorbis_comment_add_tag(&comment, "encoder", "thunder");

    vorbis_analysis_init(&state, &info);
    vorbis_block_init(&state, &block);

    srand(time(NULL));
    ogg_stream_init(&stream, rand());

    ogg_packet header_packet;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&state, &comment, &header_packet, &header_comm, &header_code);
    ogg_stream_packetin(&stream, &header_packet);
    ogg_stream_packetin(&stream, &header_comm);
    ogg_stream_packetin(&stream, &header_code);

    QString uuid    = settings->subItem(0);
    if(settings->subItemsCount() < 1) {
        uuid    = QUuid::createUuid().toString();
        settings->addSubItem(qPrintable(uuid));
    }
    QFileInfo dst(settings->absoluteDestination());

    AudioClip *clip = Engine::loadResource<AudioClip>(settings->destination());
    if(clip) {
        clip->unloadAudioData();
    }

    QFile file(dst.absolutePath() + "/" + uuid);
    if(file.open(QIODevice::WriteOnly)) {
        while(true) {
            if(ogg_stream_flush(&stream, &page) == 0) {
                break;
            }
            file.write((const char *)page.header, page.header_len);
            file.write((const char *)page.body, page.body_len);
        }

        QBuffer buffer(&m_Buffer);
        buffer.open(QIODevice::ReadOnly);

        uint32_t offset = 2 * srcChanels;

        char *ptr   = new char[BLOCK_SIZE * offset];

        bool eof    = false;
        while(!eof) {
            int64_t bytes   = buffer.read(ptr, BLOCK_SIZE * offset);
            if(bytes == 0) {
                vorbis_analysis_wrote(&state, 0);
            } else {
                float **data    = vorbis_analysis_buffer(&state, bytes / offset);
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
                        file.write((const char *)page.header, page.header_len);
                        file.write((const char *)page.body, page.body_len);

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

    ogg_stream_clear    (&stream);
    vorbis_block_clear  (&block);
    vorbis_dsp_clear    (&state);
    vorbis_comment_clear(&comment);
    vorbis_info_clear   (&info);

    VariantList header;
    header.push_back(uuid.toStdString());
    header.push_back(settings->stream());
    result[HEADER]  = header;

    return result;
}

IConverterSettings *AudioConverter::createSettings() const {
    return new AudioImportSettings();
}

void AudioConverter::onBufferReady() {
    QAudioBuffer buffer = m_pDecoder->read();

    m_Buffer.append(buffer.constData<char>(), buffer.byteCount());
}

void AudioConverter::onFinished() {
    m_pLoop->exit();
}

bool AudioConverter::readOgg(IConverterSettings *settings, uint32_t &channels) {
    OggVorbis_File vorbisFile;
    if(ov_fopen(settings->source(), &vorbisFile) < 0) {
        return false;
    }

    vorbis_info *info   = ov_info(&vorbisFile, -1);
    channels    = info->channels;

    char out[BLOCK_SIZE];
    while(true) {
        int32_t	section = 0;
        int32_t result  = 0;
        while(result < BLOCK_SIZE) {
            int32_t length  = ov_read(&vorbisFile, (char *)(out + result), BLOCK_SIZE - result, 0, 2, 1, &section);
            if(length <= 0) {
                return true;
            }
            result += length;
        }
        m_Buffer.append(out, result);
    }
}
