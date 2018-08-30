#include "components/audiosource.h"

#include <AL/al.h>

#include <components/actor.h>

#define CLIP        "Clip"
#define BUFFER_SIZE 65536

AudioSource::AudioSource() :
        m_pClip(nullptr),
        m_Format(0),
        m_Loop(false),
        m_AutoPlay(false),
        m_pData(nullptr),
        m_Current(0),
        m_PositionSamples(0) {

    alGenSources(1, &m_ID);

    alGenBuffers(2, m_Buffers);
}

AudioSource::~AudioSource() {
    alSourceStop(m_ID);

    alDeleteBuffers(2, m_Buffers);
    alDeleteSources(1, &m_ID);
}

void AudioSource::update() {
    Actor &a    = actor();

    alSourcefv(m_ID, AL_POSITION,   a.position().v);

    if(m_pClip && m_pClip->isStream()) {
        int processed;
        alGetSourcei(m_ID, AL_BUFFERS_PROCESSED, &processed);

        switch(processed) {
            case 1: {
                int offset;
                alGetSourcei(m_ID, AL_SAMPLE_OFFSET , &offset);
                m_PositionSamples  += offset;

                alSourceUnqueueBuffers(m_ID, 1, &m_Buffers[m_Current]);
                uint32_t size   = m_pClip->readData(m_pData, BUFFER_SIZE, -1);
                if(size > 0 || (size == 0 && m_Loop)) {
                    alBufferData(m_Buffers[m_Current], m_Format, m_pData, size, m_pClip->frequency());
                    alSourceQueueBuffers(m_ID, 1, &m_Buffers[m_Current]);
                    if(size < BUFFER_SIZE && m_Loop) {
                        m_PositionSamples   = 0;
                    }
                } else {
                    int queued;
                    alGetSourcei(m_ID, AL_BUFFERS_QUEUED, &queued);
                    if(queued == 0) {
                        m_PositionSamples   = 0;
                    }
                }
                m_Current   = 1 - m_Current;
            } break;
            case 2: { // End of clip
                alSourceUnqueueBuffers(m_ID, 2, m_Buffers);
                m_Current   = 0;
            } break;
            default: break;
        }
    }
}

void AudioSource::start() {
    if(m_AutoPlay) {
        play();
    }
}

void AudioSource::play() {
    alSourcei(m_ID, AL_LOOPING, m_Loop);

    m_PositionSamples   = 0;

    if(m_pData) {
        delete m_pData;
    }

    if(m_pClip) {
        uint32_t size;
        if(m_pClip->isStream()) {
            m_pData = new uint8_t[BUFFER_SIZE];
            size    = m_pClip->readData(m_pData, BUFFER_SIZE, m_PositionSamples);
            alBufferData(m_Buffers[0], m_Format, m_pData, size, m_pClip->frequency());
            size	= m_pClip->readData(m_pData, BUFFER_SIZE, m_PositionSamples);
            alBufferData(m_Buffers[1], m_Format, m_pData, size, m_pClip->frequency());

            alSourceQueueBuffers(m_ID, 2, m_Buffers);
        } else {
            size    = (m_pClip->duration() + 0.5) * m_pClip->channels() * m_pClip->frequency() * 2;
            m_pData = new uint8_t[size];
            int32_t length  = m_pClip->readData(m_pData, size, m_PositionSamples);
            alBufferData(m_Buffers[0], m_Format, m_pData, length, m_pClip->frequency());

            alSourcei(m_ID, AL_BUFFER, m_Buffers[0]);
        }

        alSourcePlay(m_ID);
    }
}

void AudioSource::stop() {
    alSourceStop(m_ID);
}

AudioClip *AudioSource::clip() const {
    return m_pClip;
}

void AudioSource::setClip(AudioClip *clip) {
    m_pClip = clip;
    if(m_pClip) {
        switch(m_pClip->channels()) {
            case 2: {
                m_Format    = AL_FORMAT_STEREO16;
            } break;
            default: {
                m_Format    = AL_FORMAT_MONO16;
            } break;
        }
    }
}

void AudioSource::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(CLIP);
        if(it != data.end()) {
            setClip(Engine::loadResource<AudioClip>((*it).second.toString()));
        }
    }
}

VariantMap AudioSource::saveUserData() const {
    VariantMap result   = Component::saveUserData();
    {
        result[CLIP]    = Engine::reference(clip());
    }
    return result;
}

bool AudioSource::autoPlay() const {
    return m_AutoPlay;
}

void AudioSource::setAutoPlay(bool play) {
    m_AutoPlay  = play;
}

bool AudioSource::loop() const {
    return m_Loop;
}

void AudioSource::setLoop(bool loop) {
    m_Loop  = loop;
}
