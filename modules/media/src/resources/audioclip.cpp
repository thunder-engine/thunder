#include "resources/audioclip.h"

#include <vorbis/vorbisfile.h>
#include <log.h>

#define HEADER      "Header"

AudioClip::AudioClip() :
        m_Stream(false),
        m_SizeFlag(false),
        m_Frequency(0),
        m_Channels(0),
        m_Duration(0),
        m_pVorbisFile(new OggVorbis_File()),
        m_pFile(nullptr),
        m_pClip(nullptr) {

    m_pFile = Engine::file();
}

AudioClip::~AudioClip() {

}
/*!
    Returns the number of audio channels.
*/
uint32_t AudioClip::channels() const {
    return m_Channels;
}
/*!
    Returns the duration of audio clip.
*/
uint32_t AudioClip::duration() const {
    return m_Duration;
}
/*!
    Returns frequency of audio clip in Hz.
*/
uint32_t AudioClip::frequency() const {
    return m_Frequency;
}
/*!
    \internal This is an internal function and must not be called manually.
*/
uint32_t AudioClip::readData(uint8_t *out, uint32_t size, int32_t offset) {
    if(offset != -1) {
        ov_raw_seek(m_pVorbisFile, offset);
    }

    int32_t	section = 0;
    int32_t result  = 0;

    while(result < size) {
        int32_t length  = ov_read(m_pVorbisFile, (char *)(out + result), size - result, 0, 2, 1, &section);
        if(length <= 0) {
            break;
        }
        result += length;
    }
    return result;
}
/*!
    Returns true in case of the audio clip is streamed from disk; otherwise returns false.
*/
bool AudioClip::isStream() const {
    return m_Stream;
}
/*!
    \internal This is an internal function and must not be called manually.
*/
bool AudioClip::loadAudioData() {
    m_pClip     = m_pFile->fopen(m_Path.c_str(), "r");
    if(m_pClip) {
        ov_callbacks callbacks = {
            &read,
            &seek,
            &close,
            &tell
        };

        if(ov_open_callbacks(this, m_pVorbisFile, nullptr, 0, callbacks) < 0) {
            return false;
        }

        vorbis_info *info = ov_info(m_pVorbisFile, -1);

        m_Frequency = info->rate;
        m_Channels = info->channels;

        m_Duration = ov_time_total(m_pVorbisFile, -1);

        return true;
    }
    return false;
}
/*!
    \internal This is an internal function and must not be called manually.
*/
bool AudioClip::unloadAudioData() {
    return (ov_clear(m_pVorbisFile) == 0);
}

size_t AudioClip::read(void *ptr, size_t size, size_t nmemb, void *datasource) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object->m_pFile && object->m_pClip) {
        return object->m_pFile->fread(ptr, size, nmemb, object->m_pClip);
    }
    return 0;
}

int AudioClip::seek(void *datasource, int64_t offset, int whence) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object->m_pFile && object->m_pClip) {
        if(whence == SEEK_END) { // Physfs workaround
            object->m_SizeFlag  = true;
        }
        return object->m_pFile->fseek(object->m_pClip, offset);
    }
    return 0;
}

int AudioClip::close(void *datasource) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object->m_pFile && object->m_pClip) {
        return object->m_pFile->fclose(object->m_pClip);
    }
    return 0;
}

long AudioClip::tell(void *datasource) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object->m_pFile && object->m_pClip) {
        if(object->m_SizeFlag) { // Physfs workaround
            object->m_SizeFlag = false;
            return object->m_pFile->fsize(object->m_pClip);
        } else {
            return object->m_pFile->ftell(object->m_pClip);
        }

    }
    return 0;
}

/*!
    \internal
*/
void AudioClip::loadUserData(const VariantMap &data) {
    auto it = data.find(HEADER);
    if(it != data.end()) {
        VariantList header = (*it).second.value<VariantList>();
        auto i = header.begin();
        m_Path = (*i).toString();
        i++;
        m_Stream = (*i).toBool();
        i++;

        loadAudioData();
    }
}
/*!
    \internal
*/
VariantMap AudioClip::saveUserData() const {
    VariantMap result;

    VariantList header;
    header.push_back(m_Path);
    header.push_back(m_Stream);
    result[HEADER]  = header;

    return result;
}
