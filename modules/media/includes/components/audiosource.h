#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include "nativebehaviour.h"
#include "media.h"

class AudioClip;

class MEDIA_EXPORT AudioSource : public NativeBehaviour {
    A_OBJECT(AudioSource, NativeBehaviour, Components/Audio)

    A_PROPERTIES(
        A_PROPERTYEX(AudioClip *, clip, AudioSource::clip, AudioSource::setClip, "editor=Asset"),
        A_PROPERTY(bool, autoPlay, AudioSource::autoPlay, AudioSource::setAutoPlay),
        A_PROPERTY(bool, loop, AudioSource::loop, AudioSource::setLoop)
    )
    A_METHODS(
        A_METHOD(void, AudioSource::play),
        A_METHOD(void, AudioSource::stop)
    )

public:
    AudioSource();
    ~AudioSource();

    void play();

    void stop();

    AudioClip *clip() const;
    void setClip(AudioClip *clip);

    bool autoPlay() const;
    void setAutoPlay(bool play);

    bool loop() const;
    void setLoop(bool loop);

private:
    void start() override;

    void update() override;

protected:
    AudioClip *m_clip;

    std::vector<uint8_t> m_data;

    uint32_t m_format;

    uint32_t m_buffers[2];

    uint32_t m_id;

    uint8_t m_current;

    bool m_loop;

    bool m_autoPlay;

};

#endif // AUDIOSOURCE_H
