/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <resource.h>
#include <media.h>

class OggVorbis_File;

class MEDIA_EXPORT AudioClip : public Resource {
    A_OBJECT(AudioClip, Resource, Resources)

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

private:
    static size_t read(void *ptr, size_t size, size_t nmemb, void *datasource);
    static int seek(void *datasource, int64_t offset, int whence);
    static int close(void *datasource);
    static long tell(void *datasource);

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    File m_clip;

    OggVorbis_File *m_vorbisFile;

    uint32_t m_frequency;

    uint32_t m_channels;

    uint32_t m_duration;

    bool m_stream;

    bool m_sizeFlag;

};

#endif // AUDIOCLIP_H
