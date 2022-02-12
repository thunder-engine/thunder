#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include "nativebehaviour.h"

#include "resources/audioclip.h"

class MEDIA_EXPORT AudioSource : public NativeBehaviour {
    A_REGISTER(AudioSource, NativeBehaviour, Components/Audio)

    A_PROPERTIES(
        A_PROPERTY(AudioClip *, clip, AudioSource::clip, AudioSource::setClip),
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

    void loadUserData(const VariantMap &data) override;

    VariantMap saveUserData() const override;

protected:
    AudioClip *m_pClip;

    uint32_t m_ID;

    uint32_t m_Format;

    uint32_t m_Buffers[2];

    uint32_t m_PositionSamples;

    bool m_Loop;

    bool m_AutoPlay;

    uint8_t *m_pData;

    uint8_t m_Current;

};

#endif // AUDIOSOURCE_H
