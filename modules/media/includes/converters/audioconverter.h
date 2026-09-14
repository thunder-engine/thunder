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
#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <editor/assetconverter.h>

#include <vorbis/vorbisenc.h>

class AudioImportSettings : public AssetConverterSettings {
    A_OBJECT(AudioImportSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTY(bool, streamed, AudioImportSettings::stream, AudioImportSettings::setStream),
        A_PROPERTY(bool, forceMono, AudioImportSettings::mono, AudioImportSettings::setMono),
        A_PROPERTY(float, quality, AudioImportSettings::quality, AudioImportSettings::setQuality)
    )

public:
    AudioImportSettings();

    bool stream() const;
    void setStream(bool stream);

    bool mono() const;
    void setMono(bool mono);

    float quality() const;
    void setQuality(float quality);

private:
    StringList typeNames() const override;

protected:
    float m_quality;

    bool m_stream;

    bool m_mono;

};

class AudioConverter : public AssetConverter {
public:
    AudioConverter();

protected:
    void init() override;

    StringList suffixes() const override { return {"mp3", "wav", "ogg"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;

    VariantMap convertResource(AudioImportSettings *, int32_t srcChanels, int32_t sampleRate, const ByteArray &buffer);

    bool readOgg(AssetConverterSettings *settings, int32_t &channels, ByteArray &buffer);

};

#endif // AUDIOCONVERTER_H
