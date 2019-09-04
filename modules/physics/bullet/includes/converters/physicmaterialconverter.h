#ifndef PHYSICMATERIALCONVERTER_H
#define PHYSICMATERIALCONVERTER_H

#include <converter.h>

#include "resources/physicmaterial.h"

class PhysicMaterialConverter : public IConverter {
public:
    QStringList suffixes() const { return {"fix"}; }
    uint32_t contentType() const { return ContentPhysicMaterial; }
    uint32_t type() const { return MetaType::type<PhysicMaterial *>(); }
    uint8_t convertFile(IConverterSettings *s);
};

#endif // PHYSICMATERIALCONVERTER_H
