#include "converters/audioconverter.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <log.h>
#include <url.h>

#include <vorbis/vorbisfile.h>

#include "resources/audioclip.h"

#define HEADER      "Header"
#define BLOCK_SIZE  1024
#define FORMAT_VERSION 1

AudioImportSettings::AudioImportSettings() :
        AssetConverterSettings(),
        m_quality(1.0f),
        m_stream(false),
        m_mono(false) {

    setVersion(FORMAT_VERSION);
}

StringList AudioImportSettings::typeNames() const {
    return { MetaType::name<AudioClip>() };
}

bool AudioImportSettings::stream() const {
    return m_stream;
}

void AudioImportSettings::setStream(bool stream) {
    if(m_stream != stream) {
        m_stream = stream;
        setModified();
    }
}

bool AudioImportSettings::mono() const {
    return m_mono;
}

void AudioImportSettings::setMono(bool mono) {
    if(m_mono != mono) {
        m_mono = mono;
        setModified();
    }
}

float AudioImportSettings::quality() const {
    return m_quality;
}

void AudioImportSettings::setQuality(float quality) {
    if(m_quality != quality) {
        m_quality = quality;
        setModified();
    }
}

AudioConverter::AudioConverter()  {

}

void AudioConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/audio.svg");
    }
}

ma_result customReadProc(ma_decoder *decoder, void *buffer, size_t bytesToRead, size_t *bytesRead) {
    FILE *fp = (FILE*)decoder->pUserData;
    *bytesRead = fread(buffer, 1, bytesToRead, fp);

    return MA_SUCCESS;
}

ma_result customSeekProc(ma_decoder *decoder, ma_int64 offset, ma_seek_origin origin) {
    FILE *fp = (FILE*)decoder->pUserData;

    int fseek_origin = (origin == ma_seek_origin_start) ? SEEK_SET : SEEK_CUR;
    if(fseek(fp, (long)offset, fseek_origin) == 0) {
        return MA_SUCCESS;
    }

    return MA_BAD_SEEK;
}

size_t read(const ByteArray &data, char *buffer, size_t maxSize, size_t &pos) {
    if(pos >= data.size()) {
        return 0;
    }

    size_t bytesToRead = MIN(maxSize, data.size() - pos);
    std::copy(data.begin() + pos, data.begin() + pos + bytesToRead, buffer);
    pos += bytesToRead;

    return bytesToRead;
}

AssetConverter::ReturnCode AudioConverter::convertFile(AssetConverterSettings *settings) {
    ByteArray buffer;

    int32_t channels = 1;
    int32_t sampleRate = 0;

    Url info(settings->source());

    if(info.suffix() == "ogg") {
        readOgg(settings, channels, buffer);
    } else {
        TString filename(settings->source());

        FILE *fp = fopen(filename.data(), "rb");
        if(!fp) {
            aError() << "Unable to open file:" << filename.data();
            return InternalError;
        }

        ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_s16, 0, 0);
        ma_decoder decoder;

        ma_result decodeResult = ma_decoder_init(customReadProc, customSeekProc, fp, &decoderConfig, &decoder);
        if(decodeResult != MA_SUCCESS) {
            aError() << "Unable to initilize decoder:" << decodeResult;
            fclose(fp);
            return InternalError;
        }

        channels = decoder.outputChannels;
        sampleRate = decoder.outputSampleRate;

        ma_uint64 totalFrames;
        decodeResult = ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);
        if(decodeResult != MA_SUCCESS) {
            aError() << "Unable to read file length";
            ma_decoder_uninit(&decoder);
            fclose(fp);
            return InternalError;
        }

        ma_uint32 sampleSize = ma_get_bytes_per_sample(decoder.outputFormat);
        ma_uint64 dataSizeInBytes = totalFrames * channels * sampleSize;

        buffer.resize(dataSizeInBytes);

        ma_uint64 framesRead;
        decodeResult = ma_decoder_read_pcm_frames(&decoder, buffer.data(), totalFrames, &framesRead);
        if(decodeResult != MA_SUCCESS || framesRead != totalFrames) {
            aError() << "Unable to decode file. Frames converted:" << framesRead << "from" << totalFrames;
            return InternalError;
        }

        ma_decoder_uninit(&decoder);
        fclose(fp);
    }

    AudioClip *clip = Engine::loadResource<AudioClip>(settings->destination());
    if(clip == nullptr) {
        clip = Engine::objectCreate<AudioClip>(settings->destination());
    }

    uint32_t uuid = settings->info().id;
    if(uuid == 0) {
        uuid = Engine::generateUUID();
        settings->info().id = uuid;
    }

    if(clip->uuid() != uuid) {
        Engine::replaceUUID(clip, uuid);
    }

    clip->loadUserData(convertResource(static_cast<AudioImportSettings *>(settings), channels, sampleRate, buffer));

    return settings->saveBinary(Engine::toVariant(clip), settings->absoluteDestination());
}

VariantMap AudioConverter::convertResource(AudioImportSettings *settings, int32_t srcChanels, int32_t sampleRate, const ByteArray &buffer) {
    VariantMap result;

    ogg_stream_state stream;
    ogg_page page;

    vorbis_dsp_state state;
    vorbis_block block;
    vorbis_info info;
    vorbis_comment comment;

    vorbis_info_init(&info);

    int32_t channels = srcChanels;
    if(settings->mono()) {
        channels = 1;
    }
    vorbis_encode_init_vbr(&info, channels, sampleRate, CLAMP(settings->quality(), 0.0, 1.0));

    vorbis_comment_init(&comment);
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

    TString path("stream");
    ResourceSystem::ResourceInfo resInfo = settings->subItem(path, true);
    settings->setSubItem(path, resInfo);

    AudioClip *clip = Engine::loadResource<AudioClip>(settings->destination());
    if(clip) {
        clip->unloadAudioData();
    }

    Url dst(settings->absoluteDestination());
    File file(dst.absoluteDir() + "/" + resInfo.uuid);
    if(file.open(File::WriteOnly)) {
        while(true) {
            if(ogg_stream_flush(&stream, &page) == 0) {
                break;
            }
            file.write(reinterpret_cast<const char *>(page.header), page.header_len);
            file.write(reinterpret_cast<const char *>(page.body), page.body_len);
        }

        int64_t offset = 2 * srcChanels;
        size_t pos = 0;

        char *ptr = new char[BLOCK_SIZE * offset];

        bool eof = false;
        while(!eof) {
            int64_t bytes = read(buffer, ptr, BLOCK_SIZE * offset, pos);
            if(bytes == 0) {
                vorbis_analysis_wrote(&state, 0);
            } else {
                float **data = vorbis_analysis_buffer(&state, bytes / offset);
                uint64_t i;
                for(i = 0; i < bytes / offset; i++) {
                    data[0][i] = ((ptr[i*offset+1] << 8) | (0x00ff & (int)ptr[i * offset])) / 32768.f;
                    if(channels > 1) {
                        data[1][i] = ((ptr[i*offset+3] << 8) | (0x00ff & (int)ptr[i*offset+2])) / 32768.f;
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
    header.push_back(resInfo.uuid);
    header.push_back(settings->stream());
    result[HEADER] = header;

    return result;
}

AssetConverterSettings *AudioConverter::createSettings() {
    return new AudioImportSettings();
}

bool AudioConverter::readOgg(AssetConverterSettings *settings, int32_t &channels, ByteArray &buffer) {
    OggVorbis_File vorbisFile;
    if(ov_fopen(settings->source().data(), &vorbisFile) < 0) {
        return false;
    }

    vorbis_info *info = ov_info(&vorbisFile, -1);
    channels = info->channels;

    char out[BLOCK_SIZE];
    while(true) {
        int32_t	section = 0;
        int32_t result = 0;
        while(result < BLOCK_SIZE) {
            int32_t length = ov_read(&vorbisFile, out + result, BLOCK_SIZE - result, 0, 2, 1, &section);
            if(length <= 0) {
                return true;
            }
            result += length;
        }
        buffer.insert(buffer.end(), out, out + result);
    }
}
