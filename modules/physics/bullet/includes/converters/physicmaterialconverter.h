#ifndef PHYSICMATERIALCONVERTER_H
#define PHYSICMATERIALCONVERTER_H

#include <assetconverter.h>

#include "resources/physicmaterial.h"

class PhysicMaterialImportSettings : public AssetConverterSettings {
public:
    PhysicMaterialImportSettings();

private:
    TString defaultIconPath(const TString &) const override;

};

class PhysicMaterialConverter : public AssetConverter {
private:
    StringList suffixes() const override { return {"fix"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) override;
    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Physical_Material.fix"; }

};

#endif // PHYSICMATERIALCONVERTER_H
