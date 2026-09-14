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
#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverterSettings : public AssetConverterSettings {
public:
    MapConverterSettings();

private:
    StringList typeNames() const override;

    bool isReadOnly() const override;

};

class MapConverter : public PrefabConverter {
    void init() override;

    StringList suffixes() const override { return { "map" }; }

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override { return nullptr; }

    TString templatePath() const override;

    bool toVersion3(Variant &variant) override;
    bool toVersion4(Variant &variant) override;
    bool toVersion5(Variant &variant) override;
};

#endif // MAPCONVERTER_H
