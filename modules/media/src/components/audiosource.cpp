#include "components/audiosource.h"

#include <AL/al.h>

#include <components/transform.h>

#include "resources/audioclip.h"

namespace {
    const char *gClip = "Clip";
}

#define BUFFER_SIZE 65536

/*!
    \class AudioSource
    \brief The AudioSource class represents a source of audio in a 3D space, handling playback of audio clips.
    \inmodule Components

    The AudioSource class provides methods to manage the playback of audio clips.
    It allows users to set audio clips, control playback, and adjust parameters like auto-play and looping.
*/

AudioSource::AudioSource() :
        m_clip(nullptr),
        m_format(0),
        m_positionSamples(0),
        m_id(0),
        m_current(0),
        m_loop(false),
        m_autoPlay(false) {

    alGenSources(1, &m_id);

    alGenBuffers(2, m_buffers);
}

AudioSource::~AudioSource() {
    alSourceStop(m_id);

    alDeleteBuffers(2, m_buffers);
    alDeleteSources(1, &m_id);
}
/*!
    \internal
    Updates the audio source, adjusting its position and handling streaming for audio clips.
*/
void AudioSource::update() {
    alSourcefv(m_id, AL_POSITION, transform()->worldPosition().v);

    if(m_clip && m_clip->isStream()) {
        int processed;
        alGetSourcei(m_id, AL_BUFFERS_PROCESSED, &processed);

        switch(processed) {
            case 1: {
                int32_t offset;
                alGetSourcei(m_id, AL_SAMPLE_OFFSET, &offset);
                m_positionSamples += offset;

                alSourceUnqueueBuffers(m_id, 1, &m_buffers[m_current]);
                uint32_t size = m_clip->readData(m_data.data(), BUFFER_SIZE, -1);
                if(size > 0 || m_loop) {
                    alBufferData(m_buffers[m_current], m_format, m_data.data(), size, m_clip->frequency());
                    alSourceQueueBuffers(m_id, 1, &m_buffers[m_current]);
                    if(size < BUFFER_SIZE && m_loop) {
                        m_positionSamples = 0;
                    }
                } else {
                    int queued;
                    alGetSourcei(m_id, AL_BUFFERS_QUEUED, &queued);
                    if(queued == 0) {
                        m_positionSamples = 0;
                    }
                }
                m_current = 1 - m_current;
            } break;
            case 2: { // End of clip
                alSourceUnqueueBuffers(m_id, 2, m_buffers);
                m_current = 0;
            } break;
            default: break;
        }
    }
}
/*!
    \internal
    Starts the audio source. If AutoPlay is enabled, it automatically starts playing.
*/
void AudioSource::start() {
    if(m_autoPlay) {
        play();
    }
}
/*!
    Plays the audio clip in the specific position in 3D space.
*/
void AudioSource::play() {
    alSourcei(m_id, AL_LOOPING, m_loop);

    m_positionSamples = 0;

    if(m_clip) {
        uint32_t size;
        if(m_clip->isStream()) {
            m_data.resize(BUFFER_SIZE);
            size = m_clip->readData(m_data.data(), BUFFER_SIZE, m_positionSamples);
            alBufferData(m_buffers[0], m_format, m_data.data(), size, m_clip->frequency());
            size = m_clip->readData(m_data.data(), BUFFER_SIZE, m_positionSamples);
            alBufferData(m_buffers[1], m_format, m_data.data(), size, m_clip->frequency());

            alSourceQueueBuffers(m_id, 2, m_buffers);
        } else {
            size = (m_clip->duration() + 0.5) * m_clip->channels() * m_clip->frequency() * 2;
            m_data.resize(size);
            int32_t length = m_clip->readData(m_data.data(), size, m_positionSamples);

            alBufferData(m_buffers[0], m_format, m_data.data(), length, m_clip->frequency());
            alSourcei(m_id, AL_BUFFER, m_buffers[0]);
        }

        alSourcePlay(m_id);
    }
}
/*!
    Stops the audio source.
*/
void AudioSource::stop() {
    alSourceStop(m_id);
}
/*!
    Returns the audio clip associated with the audio source.
*/
AudioClip *AudioSource::clip() const {
    return m_clip;
}
/*!
    Sets the audio \a clip for the audio source.
*/
void AudioSource::setClip(AudioClip *clip) {
    m_clip = clip;
    if(m_clip) {
        switch(m_clip->channels()) {
            case 2: {
                m_format = AL_FORMAT_STEREO16;
            } break;
            default: {
                m_format = AL_FORMAT_MONO16;
            } break;
        }
    }
}
/*!
    \internal
*/
void AudioSource::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gClip);
        if(it != data.end()) {
            setClip(Engine::loadResource<AudioClip>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap AudioSource::saveUserData() const {
    VariantMap result = Component::saveUserData();

    TString ref = Engine::reference(clip());
    if(!ref.isEmpty()) {
        result[gClip] = ref;
    }

    return result;
}
/*!
    Returns true if auto-play is enabled; otherwise, returns false.
*/
bool AudioSource::autoPlay() const {
    return m_autoPlay;
}
/*!
    Sets the auto \a play state.
*/
void AudioSource::setAutoPlay(bool play) {
    m_autoPlay = play;
}
/*!
    Returns true if looping is enabled; otherwise, returns false.
*/
bool AudioSource::loop() const {
    return m_loop;
}
/*!
    Sets the \a loop state.
*/
void AudioSource::setLoop(bool loop) {
    m_loop = loop;
}
