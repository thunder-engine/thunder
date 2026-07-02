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
