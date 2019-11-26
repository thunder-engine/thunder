#ifndef PHYSICMATERIALCONVERTER_H
#define PHYSICMATERIALCONVERTER_H

#include <converter.h>

#include "resources/physicmaterial.h"

class PhysicMaterialConverter : public IConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"fix"}; }
    uint32_t contentType() const Q_DECL_OVERRIDE { return ContentPhysicMaterial; }
    uint32_t type() const Q_DECL_OVERRIDE { return MetaType::type<PhysicMaterial *>(); }
    uint8_t convertFile(IConverterSettings *settings) Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Physical_Material.fix"; }
};

#endif // PHYSICMATERIALCONVERTER_H
