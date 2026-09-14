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
#ifndef PHYSICMATERIALCONVERTER_H
#define PHYSICMATERIALCONVERTER_H

#include <editor/assetconverter.h>

class PhysicMaterialImportSettings : public AssetConverterSettings {
public:
    PhysicMaterialImportSettings();

private:
    StringList typeNames() const override;

};

class PhysicMaterialConverter : public AssetConverter {
private:
    void init() override;

    StringList suffixes() const override { return {"fix"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;
    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Physical_Material.fix"; }

};

#endif // PHYSICMATERIALCONVERTER_H
