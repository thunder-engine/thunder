#ifndef PHYSICMATERIALCONVERTER_H
#define PHYSICMATERIALCONVERTER_H

#include <converter.h>

#include "resources/physicmaterial.h"

class PhysicMaterialImportSettings : public IConverterSettings {
public:
    PhysicMaterialImportSettings();
};

class PhysicMaterialConverter : public IConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"fix"}; }
    uint8_t convertFile(IConverterSettings *settings) Q_DECL_OVERRIDE;
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Physical_Material.fix"; }
};

#endif // PHYSICMATERIALCONVERTER_H
