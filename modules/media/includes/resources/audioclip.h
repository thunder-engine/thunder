#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include "engine.h"
#include "file.h"

class OggVorbis_File;

class AudioClip : public Object {
    A_REGISTER(AudioClip, Object, Resources)

public:
    AudioClip();
    virtual ~AudioClip();

    uint32_t channels() const;

    uint32_t duration() const;

    uint32_t frequency() const;

    uint32_t readData(uint8_t *out, uint32_t size, int32_t offset);

    bool isStream() const;

    bool loadAudioData();
    bool unloadAudioData();

    void loadUserData(const VariantMap &data) override;

private:
    static size_t read(void *ptr, size_t size, size_t nmemb, void *datasource);
    static int seek(void *datasource, int64_t offset, int whence);
    static int close(void *datasource);
    static long tell(void *datasource);

    VariantMap saveUserData() const override;

    bool            m_Stream;

    bool            m_SizeFlag;

    uint32_t        m_Frequency;

    uint32_t        m_Channels;

    uint32_t        m_Duration;

    OggVorbis_File *m_pVorbisFile;

    string          m_Path;

    File           *m_pFile;

    _FILE          *m_pClip;
};

#endif // AUDIOCLIP_H
