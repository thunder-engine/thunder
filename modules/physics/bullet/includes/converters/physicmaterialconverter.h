#ifndef PHYSICMATERIALCONVERTER_H
#define PHYSICMATERIALCONVERTER_H

#include <assetconverter.h>

#include "resources/physicmaterial.h"

class PhysicMaterialImportSettings : public AssetConverterSettings {
public:
    PhysicMaterialImportSettings();
};

class PhysicMaterialConverter : public AssetConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"fix"}; }
    ReturnCode convertFile(AssetConverterSettings *settings) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Physical_Material.fix"; }
};

#endif // PHYSICMATERIALCONVERTER_H
