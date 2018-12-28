#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <components/nativebehaviour.h>

#include "resources/audioclip.h"

class NEXT_LIBRARY_EXPORT AudioSource : public NativeBehaviour {
    A_REGISTER(AudioSource, NativeBehaviour, Components);

    A_PROPERTIES (
        A_PROPERTY(AudioClip *, Audio_Clip, AudioSource::clip, AudioSource::setClip),
        A_PROPERTY(bool, Auto_Play, AudioSource::autoPlay, AudioSource::setAutoPlay),
        A_PROPERTY(bool, Loop, AudioSource::loop, AudioSource::setLoop)
    );

public:
    AudioSource                 ();

    ~AudioSource                ();

    virtual void                update                  ();

    virtual void                start                   ();

    void                        play                    ();

    void                        stop                    ();

    AudioClip                  *clip                    () const;

    void                        setClip                 (AudioClip *clip);

    void                        loadUserData            (const VariantMap &data);

    VariantMap                  saveUserData            () const;

    bool                        autoPlay                () const;
    void                        setAutoPlay             (bool play);

    bool                        loop                    () const;
    void                        setLoop                 (bool loop);

protected:
    AudioClip                  *m_pClip;

    uint32_t                    m_ID;

    uint32_t                    m_Format;

    uint32_t                    m_Buffers[2];

    uint32_t                    m_PositionSamples;

    bool                        m_Loop;

    bool                        m_AutoPlay;

    uint8_t                    *m_pData;

    uint8_t                     m_Current;
};

#endif // AUDIOSOURCE_H
