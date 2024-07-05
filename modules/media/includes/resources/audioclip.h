#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <resource.h>
#include <media.h>

class OggVorbis_File;

class MEDIA_EXPORT AudioClip : public Resource {
    A_REGISTER(AudioClip, Resource, Resources)

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

    std::string m_path;

    OggVorbis_File *m_vorbisFile;

    _FILE *m_clip;

    uint32_t m_frequency;

    uint32_t m_channels;

    uint32_t m_duration;

    bool m_stream;

    bool m_sizeFlag;

};

#endif // AUDIOCLIP_H
