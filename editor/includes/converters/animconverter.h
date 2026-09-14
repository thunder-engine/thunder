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
#ifndef ANIMCONVERTER_H
#define ANIMCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/animationclip.h>

class AnimImportSettings : public AssetConverterSettings {
public:
    AnimImportSettings();

private:
    StringList typeNames() const override;

    bool isReadOnly() const override;
};

class AnimConverter : public AssetConverter {
    void init() override;

    StringList suffixes() const override { return {"anim"}; }

    ReturnCode convertFile(AssetConverterSettings *s) override;

    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Animation.anim"; }

private:
    Variant readJson(const TString &data, AssetConverterSettings *settings);
    void toVersion1(Variant &variant);
    void toVersion3(Variant &variant);
};

#endif // ANIMCONVERTER_H
