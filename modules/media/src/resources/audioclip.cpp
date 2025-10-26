#include "resources/audioclip.h"

#include <vorbis/vorbisfile.h>

namespace  {
    const char *gHeader("Header");
}

/*!
    \class AudioClip
    \brief The AudioClip class represents an audio clip, handling loading, streaming, and accessing audio data.
    \inmodule Resources

    The AudioClip class provides methods to access and manipulate audio data, supporting loading and streaming from disk.
*/

AudioClip::AudioClip() :
        m_vorbisFile(new OggVorbis_File()),
        m_clip(TString()),
        m_frequency(0),
        m_channels(0),
        m_duration(0),
        m_stream(false),
        m_sizeFlag(false) {

}

AudioClip::~AudioClip() {

}
/*!
    Returns the number of audio channels.
*/
uint32_t AudioClip::channels() const {
    return m_channels;
}
/*!
    Returns the duration of audio clip.
*/
uint32_t AudioClip::duration() const {
    return m_duration;
}
/*!
    Returns frequency of audio clip in Hz.
*/
uint32_t AudioClip::frequency() const {
    return m_frequency;
}
/*!
    \internal
    This is an internal function and must not be called manually.
*/
uint32_t AudioClip::readData(uint8_t *out, uint32_t size, int32_t offset) {
    if(offset != -1) {
        ov_raw_seek(m_vorbisFile, offset);
    }

    int32_t	section = 0;
    int32_t result  = 0;

    while(result < size) {
        int32_t length  = ov_read(m_vorbisFile, reinterpret_cast<char *>(out + result), size - result, 0, 2, 1, &section);
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
    return m_stream;
}
/*!
    \internal
    This is an internal function and must not be called manually.
*/
bool AudioClip::loadAudioData() {
    if(m_clip.open(File::ReadOnly)) {
        ov_callbacks callbacks = { &read, &seek, &close, &tell };

        if(ov_open_callbacks(this, m_vorbisFile, nullptr, 0, callbacks) < 0) {
            return false;
        }

        vorbis_info *info = ov_info(m_vorbisFile, -1);

        m_frequency = info->rate;
        m_channels = info->channels;

        m_duration = ov_time_total(m_vorbisFile, -1);

        return true;
    }
    return false;
}
/*!
    \internal
*/
bool AudioClip::unloadAudioData() {
    return (ov_clear(m_vorbisFile) == 0);
}
/*!
    \internal
*/
size_t AudioClip::read(void *ptr, size_t size, size_t nmemb, void *datasource) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object) {
        return object->m_clip.read(ptr, size, nmemb);
    }
    return 0;
}
/*!
    \internal
*/
int AudioClip::seek(void *datasource, int64_t offset, int whence) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object) {
        if(whence == SEEK_END) { // Physfs workaround
            object->m_sizeFlag  = true;
        }
        return object->m_clip.seek(offset);
    }
    return 0;
}
/*!
    \internal
*/
int AudioClip::close(void *datasource) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object) {
        object->m_clip.close();
    }
    return 0;
}
/*!
    \internal
*/
long AudioClip::tell(void *datasource) {
    AudioClip *object = static_cast<AudioClip *>(datasource);

    if(object) {
        if(object->m_sizeFlag) { // Physfs workaround
            object->m_sizeFlag = false;
            return object->m_clip.size();
        } else {
            return object->m_clip.pos();
        }

    }
    return 0;
}
/*!
    \internal
*/
void AudioClip::loadUserData(const VariantMap &data) {
    auto it = data.find(gHeader);
    if(it != data.end()) {
        VariantList header = (*it).second.value<VariantList>();
        auto i = header.begin();
        m_clip.setFileName((*i).toString());
        i++;
        m_stream = (*i).toBool();

        loadAudioData();
    }
}
/*!
    \internal
*/
VariantMap AudioClip::saveUserData() const {
    VariantMap result;

    VariantList header;
    header.push_back(m_clip.fileName());
    header.push_back(m_stream);
    result[gHeader] = header;

    return result;
}
